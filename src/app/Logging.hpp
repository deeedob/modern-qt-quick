#pragma once

#include <QtCore/QString>

namespace app {

// Logs to the console and to a new session file in `directory`, starting with the build and
// system details, and logs uncaught exceptions and fatal messages with a stack trace.
// Returns the session file's path.
QString installLogging(const QString &directory, qsizetype keepSessions = 10);

} // namespace app
