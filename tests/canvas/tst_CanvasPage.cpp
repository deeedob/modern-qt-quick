#include "CanvasStore.hpp"

#include <QtGui/QPointingDevice>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtTest/QTest>

#include <memory>

using namespace Qt::StringLiterals;
using app::canvas::CanvasStore;

// Drives CanvasPage with trackpad pinches the way macOS delivers them: as native gestures.
class tst_CanvasPage : public QObject
{
    Q_OBJECT

private:
    struct Page
    {
        CanvasStore store;
        QQuickView view;
        QQuickItem *root = nullptr;
        QQuickItem *viewport = nullptr;

        [[nodiscard]] double zoom() const { return root->property("zoom").toDouble(); }
        [[nodiscard]] QPointF scroll() const
        {
            return { viewport->property("contentX").toDouble(),
                viewport->property("contentY").toDouble() };
        }
        // The paper coordinate under a point of the viewport.
        [[nodiscard]] QPointF paperAt(QPointF position) const
        {
            return (scroll() + position) / zoom();
        }
    };

    std::unique_ptr<QPointingDevice> m_touchpad{
        QTest::createTouchDevice(QInputDevice::DeviceType::TouchPad)
    };
    ulong m_timestamp = 0;

    static bool load(Page &page)
    {
        page.view.setResizeMode(QQuickView::SizeRootObjectToView);
        page.view.setInitialProperties({
            { "store", QVariant::fromValue(&page.store) }
        });
        page.view.loadFromModule("App.Canvas", "CanvasPage");
        page.view.resize(800, 600);
        page.view.show();
        if (!QTest::qWaitForWindowExposed(&page.view))
            return false;
        page.root = page.view.rootObject();
        for (auto *item : page.root->findChildren<QQuickItem *>())
            if (item->inherits("QQuickFlickable"))
                page.viewport = item;
        return page.viewport != nullptr;
    }

    void gesture(Page &page, Qt::NativeGestureType type, QPointF inViewport, double value = 0)
    {
        const auto inWindow = page.viewport->mapToScene(inViewport);
        const auto global = page.view.mapToGlobal(inWindow);
        if (type == Qt::ZoomNativeGesture)
            QWindowSystemInterface::handleGestureEventWithRealValue(&page.view, m_timestamp++,
                m_touchpad.get(), type, value, inWindow, global, 2);
        else
            QWindowSystemInterface::handleGestureEvent(&page.view, m_timestamp++, m_touchpad.get(),
                type, inWindow, global, 2);
        QWindowSystemInterface::flushWindowSystemEvents();
    }

    // Checks after every step that the view stays on the paper and, when anchored, that the
    // paper under the fingers stays put. Together that is what makes a pinch feel smooth.
    void pinch(Page &page, QPointF at, double perStep, int steps, bool anchored)
    {
        const auto start = page.paperAt(at);
        gesture(page, Qt::BeginNativeGesture, at);
        for (int step = 0; step < steps; ++step) {
            const auto before = page.paperAt(at);
            gesture(page, Qt::ZoomNativeGesture, at, perStep);
            const auto scroll = page.scroll();
            QVERIFY2(scroll.x() >= 0 && scroll.y() >= 0,
                qPrintable(u"step %1 scrolled off the paper to (%2, %3)"_s.arg(step)
                        .arg(scroll.x())
                        .arg(scroll.y())));
            // Flickable snaps its position to whole pixels, so allow less than one screen pixel.
            const double moved = QLineF(before, page.paperAt(at)).length() * page.zoom();
            QVERIFY2(!anchored || moved < 1.0,
                qPrintable(u"step %1 moved the paper under the fingers by %2 pixels"_s.arg(step)
                        .arg(moved)));
        }
        gesture(page, Qt::EndNativeGesture, at);
        const double drift = QLineF(start, page.paperAt(at)).length() * page.zoom();
        QVERIFY2(!anchored || drift < 3.0,
            qPrintable(u"the pinch moved the paper under the fingers by %1 pixels"_s.arg(drift)));
    }

private Q_SLOTS:
    void pinchingInZoomsAroundTheFingers()
    {
        Page page;
        QVERIFY(load(page));

        pinch(page, { 300, 200 }, 0.02, 20, true);
        if (QTest::currentTestFailed())
            return;

        QCOMPARE_GT(page.zoom(), 1.3);
    }

    void pinchingOutAtTheEdgeStaysOnThePaper()
    {
        Page page;
        QVERIFY(load(page));
        pinch(page, { 300, 200 }, 0.05, 10, true);
        if (QTest::currentTestFailed())
            return;

        // Zooming out near the corner would pull the view past the paper's edge.
        pinch(page, { 20, 20 }, -0.05, 30, false);
        if (QTest::currentTestFailed())
            return;

        QCOMPARE_LT(page.zoom(), 1.0);
    }
};

QTEST_MAIN(tst_CanvasPage)
#include "tst_CanvasPage.moc"
