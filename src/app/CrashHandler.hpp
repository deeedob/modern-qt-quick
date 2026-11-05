#pragma once

#include <string>

namespace app {

// Segfaults are left to the OS crash reporter: tracing safely from a signal handler is Linux-only.
void installCrashHandler();

[[nodiscard]] std::string stackTrace();

} // namespace app
