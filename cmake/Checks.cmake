# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# Developer checks over every source registered with app_target_defaults():
#
#   check          runs every check below whose tool was found
#   check_format   clang-format, without modifying files
#   check_clangd   clangd-tidy: compiler warnings, clang-tidy and include-cleaner, as in the editor
#   check_reuse    REUSE/SPDX licensing compliance
#   all_qmllint    (from Qt) qmllint over every QML module
#   format         applies clang-format and qmlformat in place
#
# Each tool is optional: a missing tool prints an install hint and its target is skipped.

include_guard(GLOBAL)

find_program(APP_CLANG_FORMAT clang-format)
find_program(APP_CLANGD_TIDY clangd-tidy)
find_program(APP_CLANGD clangd)
find_program(APP_REUSE reuse)
if(TARGET Qt6::qmlformat)
    set(APP_QMLFORMAT "$<TARGET_FILE:Qt6::qmlformat>")
else()
    find_program(APP_QMLFORMAT qmlformat HINTS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
endif()

add_custom_target(check)

function(_app_check_target name)
    add_custom_target(${name} ${ARGN} USES_TERMINAL VERBATIM)
    add_dependencies(check ${name})
endfunction()

function(_app_create_checks)
    get_property(_targets GLOBAL PROPERTY APP_FIRST_PARTY_TARGETS)
    get_property(_sources GLOBAL PROPERTY APP_FIRST_PARTY_SOURCES)
    list(REMOVE_DUPLICATES _sources)
    set(_cpp ${_sources})
    list(FILTER _cpp INCLUDE REGEX "\\.(cpp|hpp|h|cxx|cc|mm)$")
    set(_qml ${_sources})
    list(FILTER _qml INCLUDE REGEX "\\.qml$")

    set(_format_check_commands "")
    set(_format_commands "")
    if(APP_CLANG_FORMAT)
        list(APPEND _format_check_commands COMMAND "${APP_CLANG_FORMAT}" --dry-run --Werror ${_cpp})
        list(APPEND _format_commands COMMAND "${APP_CLANG_FORMAT}" -i ${_cpp})
    else()
        message(STATUS "Checks: clang-format not found, C++ format check skipped (install LLVM)")
    endif()
    if(APP_QMLFORMAT AND _qml)
        list(APPEND _format_commands COMMAND "${APP_QMLFORMAT}" -i ${_qml})
    endif()
    if(_format_check_commands)
        _app_check_target(check_format ${_format_check_commands} COMMENT "Checking C++ formatting")
    endif()
    if(_format_commands)
        add_custom_target(format ${_format_commands} USES_TERMINAL VERBATIM COMMENT "Formatting sources")
    endif()

    if(APP_CLANGD_TIDY AND APP_CLANGD)
        cmake_host_system_information(RESULT _jobs QUERY NUMBER_OF_LOGICAL_CORES)
        # Depends on the targets so generated headers (moc, type registrations) exist.
        _app_check_target(check_clangd
            COMMAND "${APP_CLANGD_TIDY}" -p "${PROJECT_BINARY_DIR}" --clangd-executable "${APP_CLANGD}"
                    -j ${_jobs} --fail-on-severity warn ${_cpp}
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "clangd diagnostics over the first-party C++ sources"
        )
        add_dependencies(check_clangd ${_targets})
    else()
        message(STATUS "Checks: clangd-tidy or clangd not found, check_clangd skipped "
                       "(pipx install clangd-tidy; clangd ships with LLVM)")
    endif()

    if(APP_REUSE)
        _app_check_target(check_reuse
            COMMAND "${APP_REUSE}" --root "${PROJECT_SOURCE_DIR}" lint
            COMMENT "Checking REUSE compliance"
        )
    else()
        message(STATUS "Checks: reuse not found, check_reuse skipped (pipx install \"reuse[charset-normalizer]\")")
    endif()

    if(TARGET all_qmllint)
        add_dependencies(check all_qmllint)
    endif()
endfunction()

# Runs after every deferred app_target_defaults() call has registered its sources.
cmake_language(DEFER CALL _app_create_checks)
