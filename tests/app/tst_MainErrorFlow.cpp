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
