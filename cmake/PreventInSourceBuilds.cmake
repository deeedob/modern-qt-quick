# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

file(REAL_PATH "${CMAKE_SOURCE_DIR}" _app_source_dir)
file(REAL_PATH "${CMAKE_BINARY_DIR}" _app_binary_dir)
if(_app_source_dir STREQUAL _app_binary_dir)
    message(FATAL_ERROR
        "In-source builds are not allowed. Remove CMakeCache.txt and CMakeFiles/, then use a preset:\n"
        "  cmake --workflow --preset dev")
endif()
