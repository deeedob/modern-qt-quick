#include "CanvasStore.h"

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <cmath>
#include <numbers>
#include <optional>

using namespace Qt::StringLiterals;
using app::canvas::CanvasStore;
using app::canvas::PaperBackground;
using app::canvas::Stroke;

namespace {

QList<QPointF> wobblyCircle(QPointF center, double radius)
{
    QList<QPointF> points;
    for (int i = 0; i <= 40; ++i) {
        const double angle = 2.0 * std::numbers::pi * i / 40.0;
        const double wobble = (i % 2 == 0) ? 3.0 : -3.0;
        points.append(center + QPointF(std::cos(angle), std::sin(angle)) * (radius + wobble));
    }
    return points;
}

QList<QPointF> zigzag()
{
    return {
        { 0,  0  },
        { 20, 80 },
        { 40, 0  },
        { 60, 80 },
        { 80, 0  }
    };
}

} // namespace

class tst_CanvasStore : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init() { QTest::failOnWarning(); }

    void addStrokeRecordsAnEdit()
    {
        CanvasStore store;
        store.setSnapShapes(false);
        QSignalSpy edited(&store, &CanvasStore::edited);

        store.addStroke(
            {
                { 1, 2 },
                { 3, 4 }
        },
            Qt::red);

        QCOMPARE(store.sketch().strokes.size(), 1);
        QCOMPARE(store.sketch().strokes.front()->color, "#ff0000");
        QCOMPARE(store.sketch().strokes.front()->points.back(), (shapes::Point{ .x = 3, .y = 4 }));
        QVERIFY(store.canUndo());
        QCOMPARE(edited.count(), 1);
    }

    void undoAndRedoWalkTheHistory()
    {
        CanvasStore store;
        store.addStroke(zigzag(), Qt::black);
        store.addStroke(zigzag(), Qt::blue);

        store.undo();
        QCOMPARE(store.sketch().strokes.size(), 1);
        QVERIFY(store.canRedo());
        store.redo();
        QCOMPARE(store.sketch().strokes.size(), 2);
        QVERIFY(!store.canRedo());
    }

    void aNewEditDiscardsTheRedoSteps()
    {
        CanvasStore store;
        store.addStroke(zigzag(), Qt::black);
        store.undo();

        store.addStroke(zigzag(), Qt::blue);

        QVERIFY(!store.canRedo());
        QCOMPARE(store.sketch().strokes.front()->color, "#0000ff");
    }

    void clearIsUndoable()
    {
        CanvasStore store;
        QSignalSpy edited(&store, &CanvasStore::edited);
        store.clear();
        QCOMPARE(edited.count(), 0); // nothing to clear, nothing to undo

        store.addStroke(zigzag(), Qt::black);
        store.clear();
        QVERIFY(store.isEmpty());
        store.undo();
        QCOMPARE(store.sketch().strokes.size(), 1);
    }

    void loadStartsANewHistory()
    {
        CanvasStore store;
        store.addStroke(zigzag(), Qt::black);
        QSignalSpy edited(&store, &CanvasStore::edited);
        QSignalSpy changed(&store, &CanvasStore::changed);

        store.load({
            .strokes = { std::make_shared<const Stroke>(), std::make_shared<const Stroke>() },
            .page = {},
        });

        QCOMPARE(store.sketch().strokes.size(), 2);
        QVERIFY(!store.canUndo());
        QCOMPARE(changed.count(), 1);
        QCOMPARE(edited.count(), 0);
    }

    void recognizedShapesSnapAndUndoKeepsTheRawStroke()
    {
        CanvasStore store;
        const auto drawn = wobblyCircle({ 100, 100 }, 50);

        store.addStroke(drawn, Qt::black);

        QCOMPARE(store.sketch().strokes.size(), 1);
        for (const auto point : store.sketch().strokes.front()->points)
            QVERIFY(qAbs(std::hypot(point.x - 100.0, point.y - 100.0) - 53.0) < 0.5);
        store.undo();
        QCOMPARE(store.sketch().strokes.front()->points.size(), drawn.size());
    }

    void aSnapReportsTheShape()
    {
        CanvasStore store;
        QSignalSpy snapped(&store, &CanvasStore::shapeSnapped);

        store.addStroke(wobblyCircle({ 100, 100 }, 50), Qt::black);
        store.addStroke(zigzag(), Qt::black);

        QCOMPARE(snapped.count(), 1);
        QCOMPARE(snapped.front().front().value<CanvasStore::Shape>(), CanvasStore::Shape::Circle);
    }

    void unrecognizedStrokesStayAsDrawn()
    {
        CanvasStore store;
        store.addStroke(zigzag(), Qt::black);
        QCOMPARE(store.sketch().strokes.front()->points.size(), zigzag().size());
    }

    void snappingCanBeTurnedOff()
    {
        CanvasStore store;
        store.setSnapShapes(false);
        const auto drawn = wobblyCircle({ 100, 100 }, 50);
        store.addStroke(drawn, Qt::black);
        QCOMPARE(store.sketch().strokes.front()->points.size(), drawn.size());
    }

    void emptyStrokesAreIgnored()
    {
        CanvasStore store;
        store.addStroke({}, Qt::black);
        QVERIFY(!store.canUndo());
    }

    void strokesKeepTheirWidth()
    {
        CanvasStore store;
        store.setSnapShapes(false);

        store.addStroke(zigzag(), Qt::black, 9.0);

        QCOMPARE(store.sketch().strokes.front()->width, 9.0);
    }

    void wideStrokesAndPointCountsFollowHistory()
    {
        CanvasStore store;
        store.setSnapShapes(false);
        store.addStroke(zigzag(), Qt::black, 36.0);
        QCOMPARE(store.sketch().strokes.front()->width, 36.0);
        QCOMPARE(store.pointCount(), 5);
        store.undo();
        QCOMPARE(store.pointCount(), 0);
        store.redo();
        QCOMPARE(store.pointCount(), 5);
    }

    void paperPatternChangesAreUndoable()
    {
        CanvasStore store;
        store.setPaperPattern(24, 3);
        QCOMPARE(store.paperSpacing(), 24);
        QCOMPARE(store.paperThickness(), 3);
        store.undo();
        QCOMPARE(store.paperSpacing(), 32);
        QCOMPARE(store.paperThickness(), 1);
        store.redo();
        QCOMPARE(store.paperSpacing(), 24);
    }

    void pageChangesAreUndoable()
    {
        CanvasStore store;

        store.setPageSize(1200, 1600);
        store.setPaperBackground(PaperBackground::Dots);

        QCOMPARE(store.sketch().page.width, 1200);
        QCOMPARE(store.sketch().page.height, 1600);
        QCOMPARE(store.sketch().page.background, PaperBackground::Dots);
        store.undo();
        QCOMPARE(store.sketch().page.background, PaperBackground::Plain);
        store.undo();
        QCOMPARE(store.sketch().page.width, 1600);
        QCOMPARE(store.sketch().page.height, 1200);
    }

    void resizingKeepsStrokeCoordinates()
    {
        CanvasStore store;
        store.setSnapShapes(false);
        store.addStroke(zigzag(), Qt::black);
        const auto points = store.sketch().strokes.front()->points;
        store.setPageSize(800, 600);
        QVERIFY(store.sketch().strokes.front()->points == points);
        store.undo();
        QCOMPARE(store.pageWidth(), 1600);
        QVERIFY(store.sketch().strokes.front()->points == points);
    }

    void paperBackgroundsUseStableNames()
    {
        QCOMPARE(app::canvas::paperBackgroundName(PaperBackground::Plain), "plain");
        QCOMPARE(app::canvas::paperBackgroundName(PaperBackground::Dots), "dots");
        QCOMPARE(app::canvas::paperBackgroundName(PaperBackground::Ruled), "ruled");
        QCOMPARE(app::canvas::paperBackgroundFromName("dots"),
            std::optional{ PaperBackground::Dots });
        QVERIFY(!app::canvas::paperBackgroundFromName("graph"));
    }

    void historyIsBounded()
    {
        CanvasStore store;
        for (int i = 0; i < 150; ++i)
            store.addStroke(zigzag(), Qt::black);
        while (store.canUndo())
            store.undo();
        QCOMPARE_GT(store.sketch().strokes.size(), 0);
    }
};

QTEST_GUILESS_MAIN(tst_CanvasStore)
#include "tst_CanvasStore.moc"
