#include "Logging.hpp"

#include "BuildInfo.hpp"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/QSysInfo>

#include <cpptrace/cpptrace.hpp>

#include <cstdlib>
#include <exception>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcStartup, "app.startup")
Q_STATIC_LOGGING_CATEGORY(lcCrash, "app.crash")

namespace app {

namespace {

    QString stackTrace()
    {
        return QString::fromStdString(cpptrace::generate_trace(1).to_string());
    }

    // Uncaught exceptions end here. Segfaults are left to the OS crash reporter: tracing
    // safely from a signal handler is only possible on Linux.
    [[noreturn]] void onTerminate() noexcept
    {
        // Logging allocates; if that throws, std::terminate re-enters this handler.
        static bool terminating = false;
        if (terminating)
            std::abort();
        terminating = true;

        QString reason = "std::terminate called without an active exception"_L1;
        if (const auto exception = std::current_exception()) {
            try {
                std::rethrow_exception(exception);
            } catch (const std::exception &e) {
                reason = "Uncaught exception: %1"_L1.arg(QString::fromUtf8(e.what()));
            } catch (...) {
                reason = "Uncaught exception of unknown type"_L1;
            }
        }
        qCCritical(lcCrash).noquote() << reason << '\n' << stackTrace();
        std::abort();
    }

    struct LogSink
    {
        QMutex mutex;
        QFile file;
        QtMessageHandler previous = nullptr;
    };

    LogSink &sink()
    {
        // Never destroyed: Qt may still log from static destructors after main() returns.
        static auto *const instance = new LogSink;
        return *instance;
    }

    void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        auto &s = sink();
        {
            const QMutexLocker lock(&s.mutex);
            if (s.file.isOpen()) {
                auto line = qFormatLogMessage(type, context, message);
                if (type == QtFatalMsg)
                    line += '\n'_L1 + stackTrace();
                s.file.write(line.toUtf8().append('\n'));
                s.file.flush();
            }
        }
        if (s.previous)
            s.previous(type, context, message);
    }

    void removeOldSessions(QDir directory, qsizetype keep)
    {
        const auto logs = directory.entryList({ "*.log"_L1 }, QDir::Files,
            QDir::Name | QDir::Reversed);
        for (qsizetype i = keep; i < logs.size(); ++i)
            directory.remove(logs.at(i));
    }

    // With these lines, a log file from a user is enough to reproduce their setup.
    void logStartupInfo()
    {
        qCInfo(lcStartup) << buildInfo;
        qCInfo(lcStartup).noquote() << QLibraryInfo::build();
        qCInfo(lcStartup).noquote()
            << "System" << QSysInfo::prettyProductName() << QSysInfo::currentCpuArchitecture();
    }

} // namespace

void installLogging(const QDir &directory, qsizetype keepSessions)
{
    Q_ASSERT(keepSessions > 0);
    std::set_terminate(onTerminate);
    qSetMessagePattern(
        "%{time yyyy-MM-ddTHH:mm:ss.zzz} "
        "%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}"
        "%{if-critical}C%{endif}%{if-fatal}F%{endif} %{category}: %{message}"_L1);

    directory.mkpath("."_L1);
    // Make room for the session that is about to start.
    removeOldSessions(directory, keepSessions - 1);

    const auto fileName = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"_L1) + ".log"_L1;
    auto &s = sink();
    bool opened = false;
    {
        const QMutexLocker lock(&s.mutex);
        s.file.close();
        s.file.setFileName(directory.filePath(fileName));
        opened = s.file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }
    // Chain to the previous handler so the console keeps working.
    if (const auto previous = qInstallMessageHandler(handleMessage); previous != handleMessage)
        s.previous = previous;
    // Outside the lock: the warning itself passes through handleMessage.
    if (!opened)
        qWarning("Cannot open log file %ls", qUtf16Printable(s.file.fileName()));
    logStartupInfo();
}

} // namespace app
