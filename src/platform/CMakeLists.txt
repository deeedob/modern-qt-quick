# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# A private library: headers live beside the sources and are only for this application.

qt_add_library(app_platform STATIC
    BuildInfo.hpp
    CrashHandler.cpp CrashHandler.hpp
    JsonTaskRepository.cpp JsonTaskRepository.hpp
    Logging.cpp Logging.hpp
)
app_add_build_info(app_platform)
target_include_directories(app_platform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_platform
    PUBLIC Qt::Core app::core
    PRIVATE cpptrace::cpptrace
)
app_target_defaults(app_platform)
