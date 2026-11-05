# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

qt_add_qml_module(app_frontend_timer
    URI App.Frontend.Timer
    STATIC
    QML_FILES
        TimerPage.qml
)
target_link_libraries(app_frontend_timer PRIVATE Qt::Quick app_backend_timer app_frontend_ui)
app_target_defaults(app_frontend_timer)
