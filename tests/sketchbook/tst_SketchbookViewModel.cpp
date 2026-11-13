#include "SketchbookViewModel.hpp"

#include <NotificationCenter.hpp>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QTemporaryDir>
#include <QtTest/QAbstractItemModelTester>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;
using app::canvas::CanvasStore;
using app::canvas::Stroke;
using app::sketchbook::SketchbookViewModel;
using app::sketchbook::SketchListModel;
using app::sketchbook::SketchStorage;

namespace {

QList<QPointF> zigzag()
{
    return {
        { 0,  0  },
        { 20, 80 },
        { 40, 0  },
        { 60, 80 }
    };
}

QVariant data(SketchbookViewModel &viewModel, int row, SketchListModel::Role role)
{
    return viewModel.model()->index(row).data(role);
}

std::vector<app::sketchbook::SketchEntry> entries(SketchStorage &storage)
{
    QSignalSpy listed(&storage, &SketchStorage::listed);
    storage.list();
    if (listed.isEmpty() && !listed.wait())
        return {};
    return listed.front().front().value<std::vector<app::sketchbook::SketchEntry>>();
}

void ignoreSaveFailure()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot save.*"_s));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Save sketch failed.*"_s));
}

app::sketchbook::SketchDocument load(SketchStorage &storage, const QString &id,
    quint64 requestId = 1)
{
    QSignalSpy loaded(&storage, &SketchStorage::loaded);
    storage.load(id, requestId);
    if (loaded.isEmpty() && !loaded.wait())
        return {};
    if (!loaded.front().at(3).toString().isEmpty())
        return {};
    return loaded.front().at(2).value<app::sketchbook::SketchDocument>();
}

} // namespace

class tst_SketchbookViewModel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void listsSavedSketchesNewestFirst()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        storage.save("old"_L1, { .name = "Old", .strokes = {}, .page = {} });
        storage.save("new"_L1, { .name = "New", .strokes = {}, .page = {} });
        QTRY_COMPARE(saved.count(), 2);
        CanvasStore canvas;

        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QAbstractItemModelTester tester(viewModel.model(),
            QAbstractItemModelTester::FailureReportingMode::QtTest);

        QTRY_VERIFY(viewModel.initialized());
        QCOMPARE(viewModel.model()->rowCount(), 2);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "New"_L1);
    }

    void createAddsAndOpensAnEmptySketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        canvas.load({ .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QSignalSpy opened(&viewModel, &SketchbookViewModel::opened);

        viewModel.create();

        QTRY_VERIFY(viewModel.initialized());
        QCOMPARE(opened.count(), 1);
        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);
        QVERIFY(canvas.isEmpty());
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        QCOMPARE(entries(storage).size(), 1);
    }

    void failedSaveIsReportedAndRetainsTheLatestSketch()
    {
        QTemporaryDir dir;
        QFile blocker(dir.filePath("blocker"_L1));
        QVERIFY(blocker.open(QIODevice::WriteOnly));
        blocker.close();
        SketchStorage storage(dir.filePath("blocker/child"_L1));
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QSignalSpy mutations(&storage, &SketchStorage::mutationFinished);
        ignoreSaveFailure();
        viewModel.create();

        QTRY_COMPARE(notifications.count(), 1);
        QVERIFY(viewModel.hasUnsavedChanges());
        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);
        auto *notice = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(notice);
        QVERIFY(!notice->message().isEmpty());

        mutations.clear();
        ignoreSaveFailure();
        canvas.addStroke(zigzag(), Qt::black);
        QTRY_COMPARE(mutations.count(), 1);
        QCOMPARE(mutations.front().at(2).value<SketchStorage::Mutation>(),
            SketchStorage::Mutation::Save);
        QVERIFY(!mutations.front().at(3).toString().isEmpty());
        QVERIFY(QFile::remove(dir.filePath("blocker"_L1)));
        auto *recovery = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(recovery);
        QCOMPARE(recovery->actions().front().text, "Retry"_L1);
        recovery->choose(recovery->actions().front().id);
        QTRY_VERIFY(!viewModel.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.size(), 1);
        QCOMPARE(stored.front().strokeCount, 1);
    }

    void canvasEditsAreSavedToTheOpenSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();

        canvas.addStroke(zigzag(), Qt::black);

        QTRY_VERIFY(!viewModel.hasUnsavedChanges());
        QCOMPARE(entries(storage).front().strokeCount, 1);
        QCOMPARE(data(viewModel, 0, SketchListModel::StrokeCountRole), 1);
    }

    void openLoadsTheSketchIntoTheCanvas()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1,
            { .name = "A", .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QSignalSpy opened(&viewModel, &SketchbookViewModel::opened);
        QTRY_VERIFY(viewModel.initialized());

        viewModel.open(0);

        QTRY_COMPARE(opened.count(), 1);
        QCOMPARE(canvas.sketch().strokes.size(), 1);
        QCOMPARE(viewModel.openName(), "A"_L1);
    }

    void newerOpenIntentSupersedesAnEarlierLoad()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1,
            { .name = "A", .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        storage.save("b"_L1,
            {
                .name = "B",
                .strokes = { std::make_shared<const Stroke>(), std::make_shared<const Stroke>() },
                .page = {}
        });
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QSignalSpy opened(&viewModel, &SketchbookViewModel::opened);
        QTRY_VERIFY(viewModel.initialized());

        QCOMPARE(viewModel.model()->rowCount(), 2);
        const int rowA = data(viewModel, 0, SketchListModel::NameRole) == "A"_L1 ? 0 : 1;
        QCOMPARE(data(viewModel, rowA, SketchListModel::NameRole), "A"_L1);
        QCOMPARE(data(viewModel, 1 - rowA, SketchListModel::NameRole), "B"_L1);
        viewModel.open(1 - rowA);
        viewModel.open(rowA);

        QTRY_COMPARE(opened.count(), 1);
        QCOMPARE(viewModel.openName(), "A"_L1);
        QCOMPARE(canvas.sketch().strokes.size(), 1);
    }

    void createSupersedesAPendingOpen()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1,
            { .name = "A", .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QTRY_VERIFY(viewModel.initialized());

        viewModel.open(0);
        viewModel.create();

        QTRY_VERIFY(!storage.hasUnsavedChanges());
        QCOMPARE(viewModel.openName(), "Sketch 2"_L1);
        QVERIFY(canvas.isEmpty());
    }

    void renameUpdatesTheListAndTheFile()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();

        viewModel.rename(0, "  Garden  "_L1);
        viewModel.rename(0, "   "_L1);

        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "Garden"_L1);
        QCOMPARE(viewModel.openName(), "Garden"_L1);
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        QCOMPARE(entries(storage).front().name, "Garden"_L1);
    }

    void renameOpenKeepsCanvasEditsQueuedAfterTheRename()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        viewModel.renameOpen("Canvas title"_L1);
        canvas.addStroke(zigzag(), Qt::black);

        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.front().name, "Canvas title"_L1);
        QCOMPARE(stored.front().strokeCount, 1);
    }

    void failedRenameRollsBackUnlessSuperseded_data()
    {
        QTest::addColumn<int>("renameCount");
        QTest::addColumn<bool>("edit");
        QTest::newRow("missing source") << 1 << false;
        QTest::newRow("consecutive renames") << 2 << false;
        QTest::newRow("superseding edit") << 2 << true;
    }

    void failedRenameRollsBackUnlessSuperseded()
    {
        QFETCH(int, renameCount);
        QFETCH(bool, edit);
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.size(), 1);
        QVERIFY(QFile::remove(dir.filePath(stored.front().id + ".json"_L1)));
        if (!edit)
            QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Rename sketch failed.*"_s));

        for (int rename = 1; rename <= renameCount; ++rename)
            viewModel.renameOpen(u"Title %1"_s.arg(rename));
        if (edit)
            canvas.addStroke(zigzag(), Qt::black);

        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto expectedName = edit ? u"Title %1"_s.arg(renameCount) : u"Sketch 1"_s;
        QCOMPARE(viewModel.openName(), expectedName);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), expectedName);
        QCOMPARE(notifications.count(), edit ? 0 : 1);
        if (edit) {
            const auto document = load(storage, stored.front().id);
            QCOMPARE(QString::fromStdString(document.name), expectedName);
            QCOMPARE(document.strokes.size(), 1);
        } else {
            QVERIFY(!QFile::exists(dir.filePath(stored.front().id + ".json"_L1)));
        }
    }

    void renameAndDuplicateReadTheLatestRetainedDocument()
    {
        QTemporaryDir dir;
        QFile blocker(dir.filePath("blocker"_L1));
        QVERIFY(blocker.open(QIODevice::WriteOnly));
        blocker.close();
        SketchStorage storage(dir.filePath("blocker/child"_L1));
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        QSignalSpy mutations(&storage, &SketchStorage::mutationFinished);
        ignoreSaveFailure();
        viewModel.create();
        QTRY_COMPARE(notifications.count(), 1);
        mutations.clear();
        ignoreSaveFailure();
        canvas.addStroke(zigzag(), Qt::black);
        QTRY_COMPARE(mutations.count(), 1);
        QCOMPARE(mutations.front().at(2).value<SketchStorage::Mutation>(),
            SketchStorage::Mutation::Save);
        QVERIFY(!mutations.front().at(3).toString().isEmpty());
        mutations.clear();

        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot save.*"_s));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Rename sketch failed.*"_s));
        viewModel.rename(0, "Latest"_L1);
        QTRY_COMPARE(mutations.count(), 1);
        QCOMPARE(mutations.front().at(2).value<SketchStorage::Mutation>(),
            SketchStorage::Mutation::Rename);
        QVERIFY(!mutations.front().at(3).toString().isEmpty());
        mutations.clear();
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot save.*"_s));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Duplicate sketch failed.*"_s));
        viewModel.duplicate(0);
        QTRY_COMPARE(mutations.count(), 1);
        QCOMPARE(mutations.front().at(2).value<SketchStorage::Mutation>(),
            SketchStorage::Mutation::Duplicate);
        QVERIFY(!mutations.front().at(3).toString().isEmpty());
        QVERIFY(QFile::remove(dir.filePath("blocker"_L1)));
        auto *recovery = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(recovery);
        QCOMPARE(recovery->actions().front().text, "Retry"_L1);
        recovery->choose(recovery->actions().front().id);
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.size(), 2);
        const auto source = std::ranges::find(stored, "Latest"_L1,
            &app::sketchbook::SketchEntry::name);
        const auto copy = std::ranges::find(stored, "Latest copy"_L1,
            &app::sketchbook::SketchEntry::name);
        QVERIFY(source != stored.end());
        QVERIFY(copy != stored.end());
        QCOMPARE(source->strokeCount, 1);
        QCOMPARE(copy->strokeCount, 1);
    }

    void duplicateCopiesTheDocumentWithoutOpeningIt()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        canvas.addStroke(zigzag(), Qt::black, 7.0);
        QTRY_VERIFY(!storage.hasUnsavedChanges());

        viewModel.duplicate(0);

        QCOMPARE(viewModel.model()->rowCount(), 2);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "Sketch 1 copy"_L1);
        QCOMPARE(data(viewModel, 0, SketchListModel::StrokeCountRole), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        QTRY_COMPARE(notifications.count(), 1);
        auto *notice = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(notice);
        QCOMPARE(notice->severity(), app::notify::Message::Severity::Success);
        QVERIFY(notice->message().contains("Sketch 1 copy"_L1));

        QSignalSpy opened(&viewModel, &SketchbookViewModel::opened);
        viewModel.open(0);
        QTRY_COMPARE(opened.count(), 1);
        QCOMPARE(canvas.sketch().strokes.size(), 1);
        QCOMPARE(canvas.sketch().strokes.front()->width, 7.0);
    }

    void removeDeletesAndClosesTheOpenSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        canvas.addStroke(zigzag(), Qt::black);

        viewModel.remove(0);

        QTRY_COMPARE(viewModel.model()->rowCount(), 0);
        QVERIFY(entries(storage).empty());
        QVERIFY(canvas.isEmpty());
        QVERIFY(viewModel.openName().isEmpty());
        QTRY_COMPARE(notifications.count(), 1);
        auto *notice = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(notice);
        QCOMPARE(notice->severity(), app::notify::Message::Severity::Success);
        QVERIFY(notice->message().contains("Sketch 1"_L1));
    }

    void failedDeleteKeepsTheSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.size(), 1);
        const QString path = QDir(dir.path()).filePath(stored.front().id + ".json"_L1);
        QVERIFY(QFile::remove(path));
        QVERIFY(QDir().mkpath(path));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot remove.*"_s));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Delete sketch failed.*"_s));
        viewModel.remove(0);

        QTRY_COMPARE(notifications.count(), 1);
        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);
    }

    void unsavedSketchCanBeDiscarded()
    {
        QTemporaryDir dir;
        QFile blocker(dir.filePath("blocker"_L1));
        QVERIFY(blocker.open(QIODevice::WriteOnly));
        blocker.close();
        SketchStorage storage(dir.filePath("blocker/child"_L1));
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        ignoreSaveFailure();
        viewModel.create();
        QTRY_COMPARE(notifications.count(), 1);

        viewModel.remove(0);

        QTRY_COMPARE(viewModel.model()->rowCount(), 0);
        QCOMPARE(notifications.count(), 1);
        auto *notice = qvariant_cast<app::notify::Message *>(notifications.data(
            notifications.index(0), app::notify::NotificationCenter::NotificationRole));
        QVERIFY(notice);
        QCOMPARE(notice->severity(), app::notify::Message::Severity::Success);
    }

    void externallyRemovedSketchCanBeDiscarded()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto stored = entries(storage);
        QCOMPARE(stored.size(), 1);
        QVERIFY(QFile::remove(QDir(dir.path()).filePath(stored.front().id + ".json"_L1)));

        viewModel.remove(0);

        QTRY_COMPARE(viewModel.model()->rowCount(), 0);
    }

    void deletionTracksTheSketchWhenRowsMove()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        app::notify::NotificationCenter notifications;
        SketchbookViewModel viewModel(storage, canvas, notifications);
        viewModel.create();
        viewModel.remove(0);
        viewModel.create();

        QTRY_COMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(viewModel.openName(), "Sketch 2"_L1);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "Sketch 2"_L1);
    }

    void pendingDeleteFailurePreservesEditsAfterViewModelDestruction()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        QString id;
        QSignalSpy saveResults(&storage, &SketchStorage::mutationFinished);
        {
            app::notify::NotificationCenter notifications;
            SketchbookViewModel viewModel(storage, canvas, notifications);
            viewModel.create();
            QTRY_VERIFY(!storage.hasUnsavedChanges());
            QCOMPARE(saveResults.count(), 1);
            saveResults.clear();
            const auto stored = entries(storage);
            QCOMPARE(stored.size(), 1);
            id = stored.front().id;
            const QString path = QDir(dir.path()).filePath(id + ".json"_L1);
            QVERIFY(QFile::remove(path));
            QVERIFY(QDir().mkpath(path));
            QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot remove.*"_s));
            viewModel.remove(0);
            QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*Cannot save.*"_s));
            canvas.addStroke(zigzag(), Qt::black);
        }
        QTRY_COMPARE(saveResults.count(), 1);
        QVERIFY(QDir().rmdir(QDir(dir.path()).filePath(id + ".json"_L1)));
        storage.retryFailedSaves();
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        const auto document = load(storage, id);
        QCOMPARE(document.strokes.size(), 1);
    }

    void pendingDeleteSuccessDoesNotRestoreEditsAfterViewModelDestruction()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        QString id;
        {
            app::notify::NotificationCenter notifications;
            SketchbookViewModel viewModel(storage, canvas, notifications);
            viewModel.create();
            QTRY_VERIFY(!storage.hasUnsavedChanges());
            const auto stored = entries(storage);
            QCOMPARE(stored.size(), 1);
            id = stored.front().id;
            viewModel.remove(0);
            canvas.addStroke(zigzag(), Qt::black);
        }
        QTRY_VERIFY(!storage.hasUnsavedChanges());
        QVERIFY(!QFile::exists(QDir(dir.path()).filePath(id + ".json"_L1)));
    }
};

QTEST_GUILESS_MAIN(tst_SketchbookViewModel)
#include "tst_SketchbookViewModel.moc"
