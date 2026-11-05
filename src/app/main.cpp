// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0
//
// The composition root: the only file that knows every concrete type and wires them together.

#include "AppController.hpp"

#include <BuildInfo.hpp>
#include <CrashHandler.hpp>
#include <JsonTaskRepository.hpp>
#include <Logging.hpp>
#include <TasksViewModel.hpp>
#include <TimerStore.hpp>

#include <QtCore/QCommandLineParser>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/QSurfaceFormat>
#include <QtQml/QQmlApplicationEngine>

#include <cstdlib>
#include <stdexcept>

using namespace Qt::StringLiterals;

// Exceptions escaping main() reach std::terminate, where the crash handler logs them.
int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    const auto &info = app::platform::buildInfo();
    QGuiApplication::setApplicationName(info.name.toString());
    QGuiApplication::setApplicationDisplayName(info.displayName.toString());
    QGuiApplication::setApplicationVersion(info.version.toString());

    // Multisampling smooths the edges of custom scene graph geometry such as ProgressRing.
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

    app::platform::installCrashHandler();
    const auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const auto logFile = app::platform::installLogging(dataDir + "/logs"_L1);
    app::platform::logStartupInfo();
    if (parser.isSet(crashTest))
        throw std::runtime_error("crash test requested with --crash-test");

    // Declared before the engine, so they outlive it: QML only holds non-owning references.
    app::platform::JsonTaskRepository repository(dataDir + "/tasks.json"_L1);
    app::backend::TasksViewModel tasks(repository);
    app::backend::TimerStore timer;
    app::AppController controller(logFile);

    QQmlApplicationEngine engine;
    QObject::connect(&controller, &app::AppController::languageChanged, &engine,
        &QQmlEngine::retranslate);
    if (!engine.setExternalSingletonInstance("App", "AppController", &controller))
        return EXIT_FAILURE;
    engine.setInitialProperties({
        { "tasks"_L1, QVariant::fromValue(&tasks) },
        { "timer"_L1, QVariant::fromValue(&timer) },
    });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("App", "Main");

    return QGuiApplication::exec();
}
