// The composition root: the only file that knows every concrete type and wires them together.

#include "AppController.hpp"
#include "BuildInfo.hpp"
#include "Logging.hpp"

#include <CanvasStore.hpp>         // demo
#include <NotificationCenter.hpp>
#include <SketchStorage.hpp>       // demo
#include <SketchbookViewModel.hpp> // demo

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/QSurfaceFormat>
#include <QtQml/QQmlApplicationEngine>

#include <cstdlib>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcMain, "app.main")

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName(app::buildInfo.name());
    QGuiApplication::setApplicationDisplayName(app::buildInfo.displayName());
    QGuiApplication::setApplicationVersion(app::buildInfo.version());

    // Multisampling smooths the edges of the strokes the canvas draws as scene graph geometry.
    auto format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    const QGuiApplication application(argc, argv);

    const QDir dataDirectory(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    const QDir logsDirectory(dataDirectory.filePath("logs"_L1));
    app::installLogging(logsDirectory);
    const QDir installedLicensesDirectory(QDir(QCoreApplication::applicationDirPath())
            .filePath(QString::fromUtf8(APP_LICENSES_RELATIVE_TO_EXECUTABLE)));
    const QDir licensesDirectory = installedLicensesDirectory.exists()
        ? installedLicensesDirectory
        : QDir(QString::fromUtf8(APP_DEV_LICENSES_DIRECTORY));

    // Declared before the engine, so they outlive it: QML only holds non-owning references.
    const auto sketchDirectory = dataDirectory.filePath("sketches"_L1); // demo
    qCInfo(lcMain) << "Sketchbook directory:" << sketchDirectory;     // demo
    app::sketchbook::SketchStorage storage(sketchDirectory);           // demo
    app::canvas::CanvasStore canvas;                                  // demo
    app::notify::NotificationCenter notifications;
    app::sketchbook::SketchbookViewModel sketchbook(storage, canvas, notifications); // demo
    QSettings settings;
    app::AppController controller({ .logs = logsDirectory, .licenses = licensesDirectory },
        settings);

    QQmlApplicationEngine engine;
    QObject::connect(&controller, &app::AppController::languageChanged, &engine,
        &QQmlEngine::retranslate);
    if (!engine.setExternalSingletonInstance("App", "AppController", &controller)) {
        qCCritical(lcMain) << "Cannot register application controller";
        return EXIT_FAILURE;
    }
    engine.setInitialProperties({
        { "notifications"_L1, QVariant::fromValue(&notifications) },
        // demo:begin
        { "sketchbook"_L1,    QVariant::fromValue(&sketchbook)    },
        { "canvas"_L1,        QVariant::fromValue(&canvas)        },
        // demo:end
    });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        [] {
            qCCritical(lcMain) << "Cannot create application window";
            QCoreApplication::exit(EXIT_FAILURE);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("App", "Main");

    return QGuiApplication::exec();
}
