// The composition root: the only file that knows every concrete type and wires them together.

#include "AppController.hpp"
#include "BuildInfo.hpp"
#include "Logging.hpp"

#include <CanvasStore.hpp>         // demo
#include <SketchStorage.hpp>       // demo
#include <SketchbookViewModel.hpp> // demo

#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/QSurfaceFormat>
#include <QtQml/QQmlApplicationEngine>

#include <cstdlib>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName(QString::fromUtf8(app::buildInfo.name));
    QGuiApplication::setApplicationDisplayName(QString::fromUtf8(app::buildInfo.displayName));
    QGuiApplication::setApplicationVersion(QString::fromUtf8(app::buildInfo.version));

    // Multisampling smooths the edges of the strokes the canvas draws as scene graph geometry.
    auto format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    const QGuiApplication application(argc, argv);

    const auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const auto logFile = app::installLogging(dataDir + "/logs"_L1);

    // Declared before the engine, so they outlive it: QML only holds non-owning references.
    app::sketchbook::SketchStorage storage(dataDir + "/sketches"_L1); // demo
    app::canvas::CanvasStore canvas;                                  // demo
    app::sketchbook::SketchbookViewModel sketchbook(storage, canvas); // demo
    app::AppController controller(logFile);

    QQmlApplicationEngine engine;
    QObject::connect(&controller, &app::AppController::languageChanged, &engine,
        &QQmlEngine::retranslate);
    if (!engine.setExternalSingletonInstance("App", "AppController", &controller))
        return EXIT_FAILURE;
    // demo:begin
    engine.setInitialProperties({
        { "sketchbook"_L1, QVariant::fromValue(&sketchbook) },
        { "canvas"_L1,     QVariant::fromValue(&canvas)     },
    });
    // demo:end
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("App", "Main");

    return QGuiApplication::exec();
}
