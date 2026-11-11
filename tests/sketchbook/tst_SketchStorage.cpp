#include "SketchStorage.hpp"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;
using app::canvas::PaperBackground;
using app::canvas::Stroke;
using app::sketchbook::SketchDocument;
using app::sketchbook::SketchStorage;

namespace {

SketchDocument twoStrokes()
{
    return {
        .name = "Garden",
        .strokes = {
            std::make_shared<const Stroke>(Stroke{
                .points = { { .x = 1, .y = 2 }, { .x = 3.5, .y = 4 } }, .color = "#ff0000" }),
            std::make_shared<const Stroke>(Stroke{ .points = { { .x = 0, .y = 0 } }, .color = "#000000" }),
        },
        .page = { .width = 1200, .height = 1600, .background = PaperBackground::Dots,
            .spacing = 24, .thickness = 3 },
    };
}

} // namespace

class tst_SketchStorage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void savedSketchesLoadBack()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy loaded(&storage, &SketchStorage::loaded);
        storage.save("a"_L1, twoStrokes());
        QTRY_COMPARE(saved.count(), 1);
        QCOMPARE(saved.front().at(3).toString(), QString());
        storage.load("a"_L1, 1);
        QTRY_COMPARE(loaded.count(), 1);
        const auto document = loaded.front().at(2).value<SketchDocument>();

        QCOMPARE(document.name, "Garden");
        QCOMPARE(document.strokes.size(), 2);
        QCOMPARE(document.strokes.front()->points.back(), (shapes::Point{ .x = 3.5, .y = 4 }));
        QCOMPARE(document.strokes.front()->color, "#ff0000");
        QCOMPARE(document.strokes.front()->width, 4.0);
        QCOMPARE(document.page.width, 1200);
        QCOMPARE(document.page.height, 1600);
        QCOMPARE(document.page.background, PaperBackground::Dots);
        QCOMPARE(document.page.spacing, 24);
        QCOMPARE(document.page.thickness, 3);
        QFile savedFile(QDir(dir.path()).filePath("a.json"_L1));
        QVERIFY(savedFile.open(QIODevice::ReadOnly));
        QVERIFY(savedFile.readAll().contains("\"background\":\"dots\""));
    }

    void backgroundNamesRoundTrip()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy loaded(&storage, &SketchStorage::loaded);
        quint64 requestId = 0;
        for (const auto &[background, name] : {
                 std::pair{ PaperBackground::Plain, "plain" },
                 std::pair{ PaperBackground::Dots,  "dots"  },
                 std::pair{ PaperBackground::Ruled, "ruled" }
        }) {
            auto document = twoStrokes();
            document.page.background = background;
            ++requestId;
            storage.save("a"_L1, document);
            QTRY_COMPARE(saved.count(), static_cast<int>(requestId));
            storage.load("a"_L1, requestId);
            QTRY_COMPARE(loaded.count(), static_cast<int>(requestId));
            const auto loadedDocument = loaded.back().at(2).value<SketchDocument>();
            QCOMPARE(loadedDocument.page.background, background);
            QFile file(QDir(dir.path()).filePath("a.json"_L1));
            QVERIFY(file.open(QIODevice::ReadOnly));
            QVERIFY(file.readAll().contains(QByteArray("\"background\":\"") + name + "\""));
        }
        QFile file(QDir(dir.path()).filePath("invalid.json"_L1));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"({"name":"Invalid","strokes":[],"page":{"background":"graph"}})");
        file.close();
        ++requestId;
        storage.load("invalid"_L1, requestId);
        QTRY_COMPARE(loaded.count(), static_cast<int>(requestId));
        QVERIFY(!loaded.back().at(3).toString().isEmpty());
    }

    void incompleteDocumentsAreRejected()
    {
        QTemporaryDir dir;
        QFile file(QDir(dir.path()).filePath("incomplete.json"_L1));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"({"name":"Incomplete","strokes":[]})");
        file.close();
        SketchStorage storage(dir.path());
        QSignalSpy loaded(&storage, &SketchStorage::loaded);

        storage.load("incomplete"_L1, 1);

        QTRY_COMPARE(loaded.count(), 1);
        QVERIFY(!loaded.front().at(3).toString().isEmpty());
    }

    void nullStrokesAreRejected()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy loaded(&storage, &SketchStorage::loaded);
        storage.save("a"_L1, twoStrokes());
        QTRY_COMPARE(saved.count(), 1);
        QFile file(QDir(dir.path()).filePath("a.json"_L1));
        QVERIFY(file.open(QIODevice::ReadOnly));
        auto json = file.readAll();
        file.close();
        const auto start = json.indexOf("\"strokes\":[");
        const auto end = json.indexOf(",\"page\"", start);
        QVERIFY(start >= 0);
        QVERIFY(end > start);
        json.replace(start, end - start, "\"strokes\":[null]");
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(file.write(json), json.size());
        file.close();

        storage.load("a"_L1, 1);

        QTRY_COMPARE(loaded.count(), 1);
        QVERIFY(loaded.front().at(3).toString().contains("invalid stroke"_L1));
    }

    void listDescribesEachSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy listed(&storage, &SketchStorage::listed);
        storage.save("a"_L1, twoStrokes());
        QTRY_COMPARE(saved.count(), 1);

        storage.list();
        QCOMPARE(listed.count(), 0);
        QTRY_COMPARE(listed.count(), 1);

        const auto
            entries = listed.front().front().value<std::vector<app::sketchbook::SketchEntry>>();
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.front().id, "a"_L1);
        QCOMPARE(entries.front().name, "Garden"_L1);
        QCOMPARE(entries.front().strokeCount, 2);
        QVERIFY(entries.front().modified.isValid());
    }

    void laterSavesWin()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy loaded(&storage, &SketchStorage::loaded);
        auto document = twoStrokes();
        storage.save("a"_L1, document);
        document.name = "Renamed";
        storage.save("a"_L1, document);
        QTRY_COMPARE(saved.count(), 1);

        storage.load("a"_L1, 1);

        QTRY_COMPARE(loaded.count(), 1);
        QCOMPARE(loaded.front().at(2).value<SketchDocument>().name, "Renamed");
    }

    void unreadableFilesAreReportedAndSkipped()
    {
        QTemporaryDir dir;
        QFile file(QDir(dir.path()).filePath("broken.json"_L1));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ not json");
        file.close();
        SketchStorage storage(dir.path());
        QSignalSpy listed(&storage, &SketchStorage::listed);
        QSignalSpy loaded(&storage, &SketchStorage::loaded);

        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"broken"_s));
        storage.list();
        QTRY_COMPARE(listed.count(), 1);
        storage.load("broken"_L1, 1);
        QTRY_COMPARE(loaded.count(), 1);
        QVERIFY(!loaded.front().at(3).toString().isEmpty());
        storage.load("missing"_L1, 2);
        QTRY_COMPARE(loaded.count(), 2);
        QVERIFY(!loaded.back().at(3).toString().isEmpty());
    }

    void removeDeletesTheFile()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy saved(&storage, &SketchStorage::mutationFinished);
        QSignalSpy removed(&storage, &SketchStorage::removed);
        QSignalSpy listed(&storage, &SketchStorage::listed);
        storage.save("a"_L1, twoStrokes());
        QTRY_COMPARE(saved.count(), 1);

        storage.remove("a"_L1);
        QTRY_COMPARE(removed.count(), 1);
        QCOMPARE(removed.front().at(1).toString(), QString());
        storage.list();
        QTRY_COMPARE(listed.count(), 1);
        QVERIFY(listed.front().front().value<std::vector<app::sketchbook::SketchEntry>>().empty());
    }

    void resultsReturnThroughTheEventLoop()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        QSignalSpy listed(&storage, &SketchStorage::listed);

        storage.list();

        QCOMPARE(listed.count(), 0);
        QTRY_COMPARE(listed.count(), 1);
    }

    void destructionDrainsAcceptedWritesWithoutGuiCallbacks()
    {
        QTemporaryDir dir;
        {
            auto storage = std::make_unique<SketchStorage>(dir.path());
            storage->save("a"_L1, twoStrokes());
        }

        QFile file(QDir(dir.path()).filePath("a.json"_L1));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("\"name\":\"Garden\""));
    }
};

QTEST_GUILESS_MAIN(tst_SketchStorage)
#include "tst_SketchStorage.moc"
