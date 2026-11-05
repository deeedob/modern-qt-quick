# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

qt_add_executable(app MACOSX_BUNDLE WIN32 main.cpp)
set(_packaging "${PROJECT_SOURCE_DIR}/packaging")
if(APPLE)
    target_sources(app PRIVATE "${_packaging}/app.icns")
    set_source_files_properties("${_packaging}/app.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32)
    target_sources(app PRIVATE "${_packaging}/app.rc")
endif()
set_target_properties(app PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_ICON_FILE app.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}"
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

# The About dialog shows the same artwork as the application icon.
set_source_files_properties("${_packaging}/app.svg" PROPERTIES QT_RESOURCE_ALIAS app.svg)

# The executable is a QML module too (URI App), holding the shell and the app-wide controller.
qt_add_qml_module(app
    URI App
    QML_FILES
        AboutDialog.qml
        Main.qml
    SOURCES
        AppController.cpp AppController.hpp
    RESOURCES
        "${_packaging}/app.svg"
)

target_link_libraries(app PRIVATE
    Qt::Gui
    Qt::Quick
    # Nothing calls QtSvg directly, but the deploy tool only ships the SVG icon plugin with it.
    Qt::Svg
    app_platform
    app_backend_tasks
    app_backend_timer
    # Static QML modules are registered by linking their plugins.
    app_backend_tasksplugin
    app_backend_timerplugin
    app_frontend_uiplugin
    app_frontend_tasksplugin
    app_frontend_timerplugin
)
app_target_defaults(app)

qt_add_translations(app
    TS_FILE_BASE app
    TS_FILE_DIR "${PROJECT_SOURCE_DIR}/i18n"
    # Drop strings that no longer exist in the sources instead of keeping them as "vanished".
    LUPDATE_OPTIONS -no-obsolete
)

# `cmake --install` produces a self-contained app: Qt's deploy tool copies the Qt libraries,
# plugins and QML modules next to it. CPack (cmake/Packaging.cmake) archives that tree.
include(GNUInstallDirs)
install(TARGETS app
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
if(WIN32)
    # The deploy tool knows Qt's DLLs but not vcpkg's.
    install(FILES "$<TARGET_RUNTIME_DLLS:app>" DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
qt_generate_deploy_qml_app_script(
    TARGET app
    OUTPUT_SCRIPT _deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    # Keep symbols so crash traces in the log name functions, not just addresses.
    DEPLOY_TOOL_OPTIONS $<$<PLATFORM_ID:Darwin>:-no-strip>
)
install(SCRIPT "${_deploy_script}")
