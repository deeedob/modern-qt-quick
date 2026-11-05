# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
#   app_add_build_info(<target>)
#       Generates BuildInfo.cpp, which defines app::platform::buildInfo() (declared in
#       src/platform/BuildInfo.hpp), and adds it to <target>. The values are captured when
#       CMake runs, which it does again after every commit or checkout.

include_guard(GLOBAL)

function(app_add_build_info target)
    set(APP_GIT_SHA "unknown")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short=12 HEAD
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            RESULT_VARIABLE _result OUTPUT_VARIABLE _sha
            OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
        )
        if(_result EQUAL 0)
            set(APP_GIT_SHA "${_sha}")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=no
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                OUTPUT_VARIABLE _status OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
            )
            if(NOT _status STREQUAL "")
                string(APPEND APP_GIT_SHA "-dirty")
            endif()
        endif()
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --absolute-git-dir
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            OUTPUT_VARIABLE _git_dir OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
        )
        # The reflog changes on every commit and checkout: re-run CMake so the SHA stays current.
        if(EXISTS "${_git_dir}/logs/HEAD")
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_git_dir}/logs/HEAD")
        endif()
    endif()

    set_property(GLOBAL PROPERTY APP_GIT_SHA "${APP_GIT_SHA}")

    # Honors SOURCE_DATE_EPOCH for reproducible builds.
    string(TIMESTAMP APP_BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)

    set(_dir "${CMAKE_CURRENT_BINARY_DIR}/buildinfo")
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/BuildInfo.cpp.in" "${_dir}/BuildInfo.cpp.in" @ONLY)
    # Second pass resolves $<CONFIG>, so multi-config generators get one file per configuration.
    file(GENERATE OUTPUT "${_dir}/$<CONFIG>/BuildInfo.cpp" INPUT "${_dir}/BuildInfo.cpp.in")
    target_sources(${target} PRIVATE "${_dir}/$<CONFIG>/BuildInfo.cpp")
endfunction()
