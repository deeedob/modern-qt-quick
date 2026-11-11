#pragma once

#include <QtCore/QDir>

namespace app {

void installLogging(const QDir &directory, qsizetype keepSessions = 10);

} // namespace app
