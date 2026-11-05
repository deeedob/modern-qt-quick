// The composition root: the only file that knows every concrete type and wires them together.

#include "AppController.hpp"
#include "BuildInfo.hpp"
#include "CrashHandler.hpp"
#include "Logging.hpp"

#include <CanvasStore.hpp>
#include <SketchStorage.hpp>
#include <SketchbookViewModel.hpp>

#include <QtCore/QCommandLineParser>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/QSurfaceFormat>
#include <QtQml/QQmlApplicationEngine>

#include <cstdlib>
#include <stdexcept>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    const auto &info = app::buildInfo();
    QGuiApplication::setApplicationName(info.name.toString());
    QGuiApplication::setApplicationDisplayName(info.displayName.toString());
    QGuiApplication::setApplicationVersion(info.version.toString());

    // Multisampling smooths the edges of the strokes the canvas draws as scene graph geometry.
    auto format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    const QGuiApplication application(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption crashTest("crash-test"_L1,
        "Throw an uncaught exception to test crash logging."_L1);
    crashTest.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(crashTest);
    parser.process(application);

    app::installCrashHandler();
    const auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const auto logFile = app::installLogging(dataDir + "/logs"_L1);
    app::logStartupInfo();
    // Escapes main() on purpose: the crash handler logs it with a stack trace.
    if (parser.isSet(crashTest))
        throw std::runtime_error("crash test requested with --crash-test");

    // Declared before the engine, so they outlive it: QML only holds non-owning references.
    app::sketchbook::SketchStorage storage(dataDir + "/sketches"_L1);
    app::canvas::CanvasStore canvas;
    app::sketchbook::SketchbookViewModel sketchbook(storage, canvas);
    app::AppController controller(logFile);

    QQmlApplicationEngine engine;
    QObject::connect(&controller, &app::AppController::languageChanged, &engine,
        &QQmlEngine::retranslate);
    if (!engine.setExternalSingletonInstance("App", "AppController", &controller))
        return EXIT_FAILURE;
    engine.setInitialProperties({
        { "sketchbook"_L1, QVariant::fromValue(&sketchbook) },
        { "canvas"_L1,     QVariant::fromValue(&canvas)     },
    });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("App", "Main");

    return QGuiApplication::exec();
}
