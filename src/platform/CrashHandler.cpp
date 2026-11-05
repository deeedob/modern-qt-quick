// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "CrashHandler.hpp"

#include <QtCore/QLoggingCategory>

#include <cpptrace/cpptrace.hpp>

#include <cstdlib>
#include <exception>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcCrash, "app.crash")

namespace app::platform {

namespace {

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
        qCCritical(lcCrash).noquote() << reason << '\n' << QString::fromStdString(stackTrace());
        std::abort();
    }

} // namespace

void installCrashHandler()
{
    std::set_terminate(onTerminate);
}

std::string stackTrace()
{
    return cpptrace::generate_trace(1).to_string();
}

} // namespace app::platform
