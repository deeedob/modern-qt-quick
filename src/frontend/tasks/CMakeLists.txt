# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

qt_add_qml_module(app_frontend_tasks
    URI App.Frontend.Tasks
    STATIC
    QML_FILES
        TaskDelegate.qml
        TasksPage.qml
)
target_link_libraries(app_frontend_tasks PRIVATE Qt::Quick app_backend_tasks app_frontend_ui)
app_target_defaults(app_frontend_tasks)
