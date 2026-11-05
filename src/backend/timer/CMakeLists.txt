# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# A C++-only QML module: registers the types the Timer view binds to.
# Links Qt::Qml but never Qt::Quick, so stores cannot depend on visual items.

qt_add_qml_module(app_backend_timer
    URI App.Backend.Timer
    STATIC
    SOURCES
        TimerStore.cpp TimerStore.hpp
)
target_include_directories(app_backend_timer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_backend_timer PUBLIC Qt::Qml app::core)
app_target_defaults(app_backend_timer)
