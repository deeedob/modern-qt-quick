// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <QtCore/QString>

namespace app::platform {

// Logs to the console and to a new session file in `directory`; returns that file's path.
QString installLogging(const QString &directory, qsizetype keepSessions = 10);

void logStartupInfo();

} // namespace app::platform
