# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# One test tree per layer, each with the lightest tool that can test it:
#   libs      Catch2 (no Qt at all)
#   platform  Qt Test
#   backend   Qt Test, headless (no QtQuick)
#   frontend  Qt Quick Test

set_directory_properties(PROPERTIES QT_EXCLUDE_FROM_TRANSLATION ON)
include(Catch)

function(app_add_qt_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;LIBRARIES")
    qt_add_executable(${name} ${arg_SOURCES})
    target_link_libraries(${name} PRIVATE Qt::Test ${arg_LIBRARIES})
    app_target_defaults(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_subdirectory(libs/core)
add_subdirectory(platform)
add_subdirectory(backend)
add_subdirectory(frontend)

# Smoke test of the real executable: starts, parses the command line, reports its version.
add_test(NAME app.version COMMAND app --version)
string(REPLACE "." "\\." _version_regex "${PROJECT_VERSION}")
set_tests_properties(app.version PROPERTIES
    PASS_REGULAR_EXPRESSION "${PROJECT_NAME} ${_version_regex}"
    # A Windows GUI app would otherwise show --version in a message box and block the test.
    ENVIRONMENT QT_COMMAND_LINE_PARSER_NO_GUI_MESSAGE_BOXES=1
)
