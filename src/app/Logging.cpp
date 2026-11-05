#include "Logging.hpp"

#include "BuildInfo.hpp"
#include "CrashHandler.hpp"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/QSysInfo>

#include <algorithm>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcStartup, "app.startup")

namespace app {

namespace {

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
                    line += '\n'_L1 + QString::fromStdString(stackTrace());
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

} // namespace

QString installLogging(const QString &directory, qsizetype keepSessions)
{
    Q_ASSERT(keepSessions > 0);
    qSetMessagePattern(
        "%{time yyyy-MM-ddTHH:mm:ss.zzz} "
        "%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}"
        "%{if-critical}C%{endif}%{if-fatal}F%{endif} %{category}: %{message}"_L1);

    const QDir dir(directory);
    dir.mkpath("."_L1);
    // Make room for the session that is about to start.
    removeOldSessions(dir, std::max<qsizetype>(keepSessions, 1) - 1);

    const auto fileName = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"_L1) + ".log"_L1;
    auto &s = sink();
    bool opened = false;
    {
        const QMutexLocker lock(&s.mutex);
        s.file.close();
        s.file.setFileName(dir.filePath(fileName));
        opened = s.file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }
    // Chain to the previous handler so the console keeps working.
    if (const auto previous = qInstallMessageHandler(handleMessage); previous != handleMessage)
        s.previous = previous;
    // Outside the lock: the warning itself passes through handleMessage.
    if (!opened)
        qWarning("Cannot open log file %ls", qUtf16Printable(s.file.fileName()));
    return s.file.fileName();
}

void logStartupInfo()
{
    const auto &info = buildInfo();
    qCInfo(lcStartup).noquote().nospace()
        << info.name << ' ' << info.version << " (" << info.gitSha << ", " << info.buildType
        << ", built " << info.buildTimestamp << ')';
    qCInfo(lcStartup).noquote().nospace()
        << "Compiler " << info.compiler << ", target " << info.system;
    qCInfo(lcStartup).noquote() << QLibraryInfo::build();
    qCInfo(lcStartup).noquote()
        << "System" << QSysInfo::prettyProductName() << QSysInfo::currentCpuArchitecture();
}

} // namespace app
