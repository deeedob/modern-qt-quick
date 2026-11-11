#include "SketchbookViewModel.hpp"

#include <QtCore/QDir>
#include <QtCore/QFile>
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
        storage.save("old"_L1, { .name = "Old", .strokes = {}, .page = {} });
        storage.save("new"_L1, { .name = "New", .strokes = {}, .page = {} });
        storage.waitForSaves();
        QFile old(QDir(dir.path()).filePath("old.json"_L1));
        QVERIFY(old.open(QIODevice::ReadWrite));
        QVERIFY(old.setFileTime(QDateTime::currentDateTime().addDays(-1),
            QFileDevice::FileModificationTime));
        old.close();
        CanvasStore canvas;

        SketchbookViewModel viewModel(storage, canvas);
        QAbstractItemModelTester tester(viewModel.model(),
            QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(viewModel.model()->rowCount(), 2);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "New"_L1);
    }

    void createAddsAndOpensAnEmptySketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        canvas.load({ .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        SketchbookViewModel viewModel(storage, canvas);

        viewModel.create();

        QCOMPARE(viewModel.model()->rowCount(), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);
        QVERIFY(canvas.isEmpty());
        QCOMPARE(storage.list().size(), 1);
    }

    void canvasEditsAreSavedToTheOpenSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        viewModel.create();

        canvas.addStroke(zigzag(), Qt::black);

        QCOMPARE(storage.list().front().strokeCount, 1);
        QCOMPARE(data(viewModel, 0, SketchListModel::StrokeCountRole), 1);
    }

    void openLoadsTheSketchIntoTheCanvas()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1,
            { .name = "A", .strokes = { std::make_shared<const Stroke>() }, .page = {} });
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        QSignalSpy edited(&canvas, &CanvasStore::edited);

        viewModel.open(0);

        QCOMPARE(canvas.sketch().strokes.size(), 1);
        QCOMPARE(viewModel.openName(), "A"_L1);
        QCOMPARE(edited.count(), 0);
    }

    void renameUpdatesTheListAndTheFile()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        viewModel.create();

        viewModel.rename(0, "  Garden  "_L1);
        viewModel.rename(0, "   "_L1);

        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "Garden"_L1);
        QCOMPARE(viewModel.openName(), "Garden"_L1);
        QCOMPARE(storage.list().front().name, "Garden"_L1);
    }

    void renameOpenUpdatesTheOpenSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        viewModel.create();

        viewModel.renameOpen("Canvas title"_L1);

        QCOMPARE(viewModel.openName(), "Canvas title"_L1);
        QCOMPARE(storage.list().front().name, "Canvas title"_L1);
    }

    void duplicateCopiesTheDocumentWithoutOpeningIt()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        viewModel.create();
        canvas.addStroke(zigzag(), Qt::black, 7.0);

        viewModel.duplicate(0);

        QCOMPARE(viewModel.model()->rowCount(), 2);
        QCOMPARE(data(viewModel, 0, SketchListModel::NameRole), "Sketch 1 copy"_L1);
        QCOMPARE(data(viewModel, 0, SketchListModel::StrokeCountRole), 1);
        QCOMPARE(viewModel.openName(), "Sketch 1"_L1);

        viewModel.open(0);
        QCOMPARE(canvas.sketch().strokes.size(), 1);
        QCOMPARE(canvas.sketch().strokes.front()->width, 7.0);
    }

    void removeDeletesAndClosesTheOpenSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        CanvasStore canvas;
        SketchbookViewModel viewModel(storage, canvas);
        viewModel.create();
        canvas.addStroke(zigzag(), Qt::black);

        viewModel.remove(0);

        QCOMPARE(viewModel.model()->rowCount(), 0);
        QVERIFY(storage.list().empty());
        QVERIFY(canvas.isEmpty());
        QVERIFY(viewModel.openName().isEmpty());
    }
};

QTEST_GUILESS_MAIN(tst_SketchbookViewModel)
#include "tst_SketchbookViewModel.moc"
