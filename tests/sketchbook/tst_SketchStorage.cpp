#include "SketchStorage.hpp"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;
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
    };
}

} // namespace

class tst_SketchStorage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void savedSketchesLoadBack()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());

        storage.save("a"_L1, twoStrokes());
        const auto loaded = storage.load("a"_L1);

        QVERIFY2(loaded, qPrintable(loaded.error_or({})));
        QCOMPARE(loaded->name, "Garden");
        QCOMPARE(loaded->strokes.size(), 2);
        QCOMPARE(loaded->strokes.front()->points.back(), (shapes::Point{ .x = 3.5, .y = 4 }));
        QCOMPARE(loaded->strokes.front()->color, "#ff0000");
    }

    void listDescribesEachSketch()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1, twoStrokes());

        const auto entries = storage.list();

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
        auto document = twoStrokes();
        storage.save("a"_L1, document);
        document.name = "Renamed";
        storage.save("a"_L1, document);

        QCOMPARE(storage.load("a"_L1)->name, "Renamed");
    }

    void unreadableFilesAreReportedAndSkipped()
    {
        QTemporaryDir dir;
        QFile file(QDir(dir.path()).filePath("broken.json"_L1));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ not json");
        file.close();
        SketchStorage storage(dir.path());

        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("broken"));
        QVERIFY(storage.list().empty());
        QVERIFY(!storage.load("broken"_L1));
        QVERIFY(!storage.load("missing"_L1));
    }

    void removeDeletesTheFile()
    {
        QTemporaryDir dir;
        SketchStorage storage(dir.path());
        storage.save("a"_L1, twoStrokes());

        storage.remove("a"_L1);

        QVERIFY(storage.list().empty());
    }
};

QTEST_GUILESS_MAIN(tst_SketchStorage)
#include "tst_SketchStorage.moc"
