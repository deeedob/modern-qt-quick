# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# A public library: headers under include/app/core/ are its API, everything else is private.

add_library(app_core STATIC)
add_library(app::core ALIAS app_core)

target_sources(app_core
    PRIVATE
        src/TaskSearch.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/app/core/FocusTimer.hpp
            include/app/core/Task.hpp
            include/app/core/TaskRepository.hpp
            include/app/core/TaskSearch.hpp
)
target_compile_features(app_core PUBLIC cxx_std_23)
target_link_libraries(app_core PRIVATE rapidfuzz::rapidfuzz)
app_target_defaults(app_core)
