# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# The design system: reusable, application-agnostic components, fonts and icons.
# Never imports a feature module, so it could move to its own repository unchanged.

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(app_frontend_ui
    URI App.Frontend.Ui
    STATIC
    DEPENDENCIES QtQuick
    QML_FILES
        IconButton.qml
        Theme.qml
    SOURCES
        ProgressRing.cpp ProgressRing.hpp
    RESOURCES
        fonts/InterVariable.ttf
        icons/info.svg
        icons/languages.svg
        icons/pause.svg
        icons/play.svg
        icons/plus.svg
        icons/rotate-ccw.svg
        icons/trash.svg
)
target_link_libraries(app_frontend_ui PRIVATE Qt::Quick)
app_target_defaults(app_frontend_ui)
