#include "CanvasStore.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>
#include <QtGui/QNativeGestureEvent>
#include <QtGui/QPointingDevice>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtTest/QTest>

#include <cmath>
#include <memory>
#include <numbers>

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
            { u"store"_s, QVariant::fromValue(&page.store) }
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
        QNativeGestureEvent event(type, m_touchpad.get(), 2, inWindow, inWindow, global, value, {});
        event.setTimestamp(m_timestamp++);
        QCoreApplication::sendEvent(&page.view, &event);
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
    void init() { QTest::failOnWarning(); }

    void canvasSettingsMaintainAspectRatio()
    {
        Page page;
        QTest::ignoreMessage(QtWarningMsg,
            QRegularExpression(
                uR"(Populating font family aliases took .*Replace uses of missing font family "Sans Serif".*)"_s));
        QVERIFY(load(page));
        auto *settings = page.root->findChild<QQuickItem *>(u"paperSettingsButton"_s);
        QVERIFY(settings);
        const QPointF settingsCenter(settings->width() / 2, settings->height() / 2);
        const QPoint settingsPoint = settings->mapToScene(settingsCenter).toPoint();
        QTest::mouseClick(&page.view, Qt::LeftButton, Qt::NoModifier, settingsPoint);
        auto *width = page.root->findChild<QObject *>(u"pageWidth"_s);
        auto *link = page.root->findChild<QQuickItem *>(u"aspectLink"_s);
        QVERIFY(width);
        QVERIFY(link);
        const QPointF linkCenter(link->width() / 2, link->height() / 2);
        const QPoint linkPoint = link->mapToScene(linkCenter).toPoint();
        QTest::mouseClick(&page.view, Qt::LeftButton, Qt::NoModifier, linkPoint);
        QVERIFY(link->property("checked").toBool());
        auto *input = width->property("contentItem").value<QQuickItem *>();
        QVERIFY(input);
        input->forceActiveFocus();
        QVERIFY(QMetaObject::invokeMethod(input, "selectAll"));
        QTest::keyClick(&page.view, Qt::Key_1);
        QTest::keyClick(&page.view, Qt::Key_2);
        QTest::keyClick(&page.view, Qt::Key_0);
        QTest::keyClick(&page.view, Qt::Key_0);
        QTest::keyClick(&page.view, Qt::Key_Return);
        QCOMPARE(page.store.pageWidth(), 1200);
        QCOMPARE(page.store.pageHeight(), 900);
    }

    void zoomControlStaysDiscoverable()
    {
        Page page;
        QVERIFY(load(page));
        auto *button = page.root->findChild<QQuickItem *>(u"zoomButton"_s);
        QVERIFY(button);
        QVERIFY(button->opacity() > 0);
        QVERIFY(button->property("text").toString().endsWith(u'%'));
        QVERIFY(page.root->setProperty("zoom", 2.0));
        const QPointF center(button->width() / 2, button->height() / 2);
        QTest::mouseClick(&page.view, Qt::LeftButton, Qt::NoModifier,
            button->mapToScene(center).toPoint());
        QTRY_COMPARE(page.zoom(), page.root->property("fitZoom").toDouble());
    }

    void aSnappedShapeShowsAHint()
    {
        Page page;
        QVERIFY(load(page));
        QList<QPointF> circle;
        for (int i = 0; i <= 40; ++i) {
            const double angle = 2.0 * std::numbers::pi * i / 40.0;
            circle.append(QPointF(300, 300) + (QPointF(std::cos(angle), std::sin(angle)) * 80));
        }

        page.store.addStroke(circle, Qt::black);

        auto *hint = page.root->findChild<QQuickItem *>(u"snapHint"_s);
        QVERIFY(hint);
        QCOMPARE(hint->property("text").toString(), u"Snapped to a circle"_s);
        QTRY_VERIFY(hint->opacity() > 0.5);
    }

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

    void fitPageUsesTheReportedZoom()
    {
        Page page;
        QVERIFY(load(page));

        QVERIFY(QMetaObject::invokeMethod(page.root, "fitPage"));

        QCOMPARE(page.zoom(), page.root->property("fitZoom").toDouble());
    }
};

QTEST_MAIN(tst_CanvasPage)
#include "tst_CanvasPage.moc"
