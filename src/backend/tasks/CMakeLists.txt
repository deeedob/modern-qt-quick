# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# A C++-only QML module: registers the types the Tasks views bind to.
# Links Qt::Qml but never Qt::Quick, so view models cannot depend on visual items.

qt_add_qml_module(app_backend_tasks
    URI App.Backend.Tasks
    STATIC
    SOURCES
        TaskListModel.cpp TaskListModel.hpp
        TasksViewModel.cpp TasksViewModel.hpp
)
target_include_directories(app_backend_tasks PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_backend_tasks PUBLIC Qt::Qml Qt::QmlModels app::core)
app_target_defaults(app_backend_tasks)
