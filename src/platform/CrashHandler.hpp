// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <string>

namespace app::platform {

// Segfaults are left to the OS crash reporter: tracing safely from a signal handler is Linux-only.
void installCrashHandler();

[[nodiscard]] std::string stackTrace();

} // namespace app::platform
