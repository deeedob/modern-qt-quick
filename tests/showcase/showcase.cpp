#include <AppController.hpp>
#include <BuildInfo.hpp>
#include <CanvasStore.hpp>
#include <SketchStorage.hpp>
#include <SketchbookViewModel.hpp>

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRendererInterface>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

namespace {

QQuickItem *button(QObject *root, const QString &text)
{
    for (auto *item : root->findChildren<QQuickItem *>())
        if (item->isVisible() && item->property("text").toString() == text
            && item->property("enabled").toBool())
            return item;
    return nullptr;
}

bool click(QQuickWindow *window, QQuickItem *item)
{
    if (!item)
        return false;
    const auto position = item->mapToScene(QPointF(item->width() / 2, item->height() / 2));
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, position.toPoint());
    QTest::qWait(500);
    return true;
}

bool save(QQuickWindow *window, const QDir &output, const QString &name)
{
    QTest::qWait(500);
    const auto image = window->grabWindow();
    return !image.isNull() && image.save(output.filePath(name));
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(app::buildInfo.name());
    QGuiApplication::setApplicationDisplayName(app::buildInfo.displayName());
    QGuiApplication::setApplicationVersion(app::buildInfo.version());
    const auto arguments = QGuiApplication::arguments();
    if (arguments.size() != 2) {
        qCritical("Usage: showcase OUTPUT_DIRECTORY");
        return 1;
    }
    const QDir output(arguments.at(1));
    if (!output.exists()) {
        qCritical("Output directory does not exist");
        return 1;
    }
    QTemporaryDir temporary;
    if (!temporary.isValid())
        return 1;
    QSettings settings(temporary.filePath("settings.ini"_L1), QSettings::IniFormat);
    settings.setValue("theme/dark"_L1, false);
    app::AppController controller({ QDir(temporary.path()), QDir(temporary.path()) }, settings);
    controller.setLanguage("en"_L1);
    app::sketchbook::SketchStorage storage(temporary.filePath("sketches"_L1));
    app::canvas::CanvasStore canvas;
    app::sketchbook::SketchbookViewModel sketchbook(storage, canvas);
    sketchbook.create();
    sketchbook.renameOpen(u"Ideas"_s);
    sketchbook.create();
    sketchbook.renameOpen(u"Shape studies"_s);
    QQmlApplicationEngine engine;
    QObject::connect(&controller, &app::AppController::languageChanged, &engine,
        &QQmlEngine::retranslate);
    if (!engine.setExternalSingletonInstance("App", "AppController", &controller))
        return 1;
    engine.setInitialProperties({
        { "sketchbook"_L1, QVariant::fromValue(&sketchbook) },
        { "canvas"_L1,     QVariant::fromValue(&canvas)     },
    });
    engine.loadFromModule("App", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (!window || !QTest::qWaitForWindowExposed(window))
        return 1;
    if (window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        qCritical("The software renderer cannot capture the canvas geometry");
        return 1;
    }
    if (!click(window, button(window, u"New sketch"_s)))
        return 1;
    canvas.setSnapShapes(false);
    canvas.addStroke(
        {
            { 150, 180 },
            { 350, 180 },
            { 250, 40  },
            { 150, 180 }
    },
        Qt::black, 5);
    canvas.addStroke(
        {
            { 480, 80  },
            { 740, 80  },
            { 740, 240 },
            { 480, 240 },
            { 480, 80  }
    },
        Qt::black, 5);
    if (!save(window, output, u"canvas-light.png"_s))
        return 1;
    controller.setDarkTheme(true);
    auto *settingsButton = window->findChild<QQuickItem *>(u"paperSettingsButton"_s);
    if (!click(window, settingsButton))
        return 1;
    QTest::mouseMove(window, QPoint(10, window->height() - 10));
    if (!save(window, output, u"canvas-dark.png"_s))
        return 1;
    if (!click(window, settingsButton))
        return 1;
    controller.setDarkTheme(false);
    if (!click(window, button(window, u"About"_s)) || !save(window, output, u"about.png"_s))
        return 1;
    QTest::keyClick(window, Qt::Key_Escape);
    QTest::qWait(500);
    if (!click(window, button(window, u"Back"_s)))
        return 1;
    controller.setLanguage("de"_L1);
    if (!save(window, output, u"sketchbook-de.png"_s))
        return 1;
    return 0;
}
