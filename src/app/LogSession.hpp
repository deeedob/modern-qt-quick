#pragma once

#include <QtCore/QDir>
#include <QtCore/QFile>

namespace app {

class LogSession
{
public:
    explicit LogSession(const QDir &directory, qsizetype keepSessions = 10);
    ~LogSession();

    LogSession(const LogSession &) = delete;
    LogSession &operator=(const LogSession &) = delete;
    LogSession(LogSession &&) = delete;
    LogSession &operator=(LogSession &&) = delete;

private:
    QFile m_file;
    QtMessageHandler m_previous = nullptr;
};

} // namespace app
