#include "AppController.hpp"
#include "SketchStorage.hpp"
#include "SketchbookViewModel.hpp"

#include <CanvasStore.hpp>

#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/qqmllist.h>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;
using app::canvas::CanvasStore;
using app::sketchbook::SketchbookViewModel;
using app::sketchbook::SketchStorage;

namespace {

struct MainFixture
{
    QTemporaryDir directory;
    SketchStorage storage{ directory.filePath("sketches"_L1) };
    CanvasStore canvas;
    SketchbookViewModel sketchbook{ storage, canvas };
    QSettings settings{ directory.filePath("settings.ini"_L1), QSettings::IniFormat };
    app::AppController controller{
        { .logs = QDir(directory.filePath("logs"_L1)), .licenses = QDir(directory.path()) },
        settings
    };
    QQmlApplicationEngine engine;
    QQuickWindow *window = nullptr;

    MainFixture()
    {
        controller.setLanguage("en"_L1);
        engine.setExternalSingletonInstance("App", "AppController", &controller);
        QObject::connect(&controller, &app::AppController::languageChanged, &engine,
            &QQmlEngine::retranslate);
        engine.setInitialProperties({
            { "sketchbook"_L1, QVariant::fromValue(&sketchbook) },
            { "canvas"_L1,     QVariant::fromValue(&canvas)     },
        });
        engine.loadFromModule("App", "Main");
        if (!engine.rootObjects().isEmpty())
            window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    }

    [[nodiscard]] QObject *errorDialog() const
    {
        return window->findChild<QObject *>("storageErrorDialog"_L1);
    }
};

} // namespace

class tst_MainErrorFlow : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        const QDir source(QString::fromUtf8(APP_SOURCE_DIR));
        const int fontId = QFontDatabase::addApplicationFont(
            source.filePath("src/controls/fonts/InterVariable.ttf"_L1));
        QVERIFY(fontId >= 0);
        QGuiApplication::setFont(
            QFont(QFontDatabase::applicationFontFamilies(fontId).constFirst()));
    }

    void init() { QTest::failOnWarning(); }

    void aboutDialogShowsBothInfoBoxesAndScrollsAtShortHeights()
    {
        MainFixture fixture;
        QVERIFY(fixture.window);
        fixture.window->hide();
        fixture.window->setWidth(360);
        fixture.window->setHeight(360);
        fixture.window->show();
        QTRY_VERIFY(fixture.window->isExposed());
        fixture.controller.setLanguage("de"_L1);
        auto *dialog = fixture.window->findChild<QObject *>("aboutDialog"_L1);
        QVERIFY(dialog);
        QVERIFY(QMetaObject::invokeMethod(dialog, "open"));
        QTRY_VERIFY(dialog->property("height").toReal() <= fixture.window->height());
        QTRY_VERIFY(dialog->property("width").toReal() <= fixture.window->width());

        auto *scroll = fixture.window->findChild<QObject *>("aboutScroll"_L1);
        QVERIFY(scroll);
        const qreal dialogX = dialog->property("x").toReal();
        const qreal dialogY = dialog->property("y").toReal();
        const qreal dialogWidth = dialog->property("width").toReal();
        const qreal dialogHeight = dialog->property("height").toReal();
        QVERIFY(dialogX >= 0);
        QVERIFY(dialogY >= 0);
        QVERIFY(dialogX + dialogWidth <= fixture.window->width());
        QVERIFY(dialogY + dialogHeight <= fixture.window->height());
        QVERIFY(fixture.window->findChild<QObject *>("buildInfoBox"_L1));
        auto *runtimeBox = fixture.window->findChild<QObject *>("runtimeInfoBox"_L1);
        QVERIFY(runtimeBox);
        QTRY_COMPARE(runtimeBox->property("title").toString(), "Laufzeit-Info"_L1);

        auto *footer = dialog->property("footer").value<QObject *>();
        QVERIFY(footer);
        const QQmlListReference buttons(footer, "contentChildren");
        QVERIFY(buttons.isValid());
        QCOMPARE(buttons.count(), 1);
        auto *closeItem = qobject_cast<QQuickItem *>(buttons.at(0));
        QVERIFY(closeItem);

        QTRY_VERIFY(scroll->property("contentHeight").toReal()
            > scroll->property("height").toReal());
        const qreal narrowX = dialog->property("x").toReal();
        const qreal narrowY = dialog->property("y").toReal();
        const qreal narrowWidth = dialog->property("width").toReal();
        const qreal narrowHeight = dialog->property("height").toReal();
        QVERIFY(narrowX >= 0);
        QVERIFY(narrowY >= 0);
        QVERIFY(narrowX + narrowWidth <= fixture.window->width());
        QVERIFY(narrowY + narrowHeight <= fixture.window->height());
        const QPointF closeTopLeft = closeItem->mapToItem(fixture.window->contentItem(), QPointF{});
        QVERIFY(closeTopLeft.x() >= 0);
        QVERIFY(closeTopLeft.y() >= 0);
        QVERIFY(closeTopLeft.x() + closeItem->width() <= fixture.window->width());
        QVERIFY(closeTopLeft.y() + closeItem->height() <= fixture.window->height());
    }

    void healthyPendingSaveWaitsBeforeClosing()
    {
        MainFixture fixture;
        QVERIFY(fixture.window);
        QTRY_VERIFY(fixture.sketchbook.initialized());

        fixture.sketchbook.create();
        QVERIFY(fixture.sketchbook.hasUnsavedChanges());
        fixture.window->close();

        QVERIFY(fixture.window->isVisible());
        QVERIFY(!fixture.errorDialog()->property("visible").toBool());
        QTRY_VERIFY(!fixture.window->isVisible());
    }

    void retryableMutationFailureKeepsCloseChoicesAvailable()
    {
        MainFixture fixture;
        QVERIFY(fixture.window);
        QTRY_VERIFY(fixture.sketchbook.initialized());

        fixture.sketchbook.create();
        QVERIFY(fixture.sketchbook.hasUnsavedChanges());
        QTest::ignoreMessage(QtCriticalMsg,
            QRegularExpression(
                u".*Sketchbook operation 1 failed for Field notes: Could not rename sketch.*"_s));
        Q_EMIT fixture.sketchbook.errorOccurred(SketchbookViewModel::Operation::Rename,
            "Field notes"_L1, "Could not rename sketch"_L1, true);
        fixture.window->close();

        auto *dialog = fixture.errorDialog();
        QVERIFY(dialog->property("visible").toBool());
        QVERIFY(fixture.window->property("closeAfterSave").toBool());
        QCOMPARE(dialog->property("title").toString(), "Rename sketch"_L1);

        QVERIFY(QMetaObject::invokeMethod(dialog, "reject"));
        QVERIFY(!dialog->property("visible").toBool());
        QVERIFY(fixture.window->isVisible());

        fixture.window->close();
        QVERIFY(dialog->property("visible").toBool());
        QVERIFY(fixture.window->property("closeAfterSave").toBool());
        QVERIFY(QMetaObject::invokeMethod(dialog, "discarded"));
        QTRY_VERIFY(!fixture.window->isVisible());
    }
};

QTEST_MAIN(tst_MainErrorFlow)
#include "tst_MainErrorFlow.moc"
