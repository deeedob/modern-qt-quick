# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)

add_executable(tst_core tst_TaskSearch.cpp tst_FocusTimer.cpp)
target_link_libraries(tst_core PRIVATE app::core Catch2::Catch2WithMain)
app_target_defaults(tst_core)
catch_discover_tests(tst_core TEST_PREFIX "core." DISCOVERY_MODE PRE_TEST)

# Same constexpr tests evaluated at runtime, so a failing check can be stepped through.
add_executable(tst_core_relaxed tst_FocusTimer.cpp)
target_link_libraries(tst_core_relaxed PRIVATE app::core Catch2::Catch2WithMain)
target_compile_definitions(tst_core_relaxed PRIVATE CATCH_CONFIG_RUNTIME_STATIC_REQUIRE)
app_target_defaults(tst_core_relaxed)
catch_discover_tests(tst_core_relaxed TEST_PREFIX "core.relaxed." DISCOVERY_MODE PRE_TEST)
