# Logging, crash handling and build info, as a library so tests can link them.
qt_add_library(app_support STATIC
    LogSession.cpp LogSession.hpp
)
app_add_build_info(app_support)
target_include_directories(app_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_support
    PUBLIC Qt::Core Qt::Qml
    PRIVATE cpptrace::cpptrace
)
app_target_defaults(app_support)

qt_add_executable(app MACOSX_BUNDLE WIN32 main.cpp)
app_set_icon(app app.svg)
set_target_properties(app PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
if(APPLE)
    set_target_properties(app PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}"
        MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()
target_compile_definitions(app PRIVATE
    APP_LICENSES_RELATIVE_TO_EXECUTABLE="${APP_LICENSES_RELATIVE_TO_EXECUTABLE}"
)

get_target_property(_build_info_header app_support APP_BUILD_INFO_HEADER)
qt_add_qml_module(app_ui
    URI App
    STATIC
    QML_FILES
        AboutDialog.qml
        Main.qml
    SOURCES
        AppController.cpp AppController.hpp
        "${_build_info_header}"
    RESOURCES
        app.svg
)
target_link_libraries(app_ui
    PUBLIC app_support Qt::Quick
    PRIVATE app_controls
    app_canvas # demo
    app_sketchbook # demo
    app_notify
)
app_target_defaults(app_ui)

qt_add_translations(app_ui
    TS_FILE_BASE app
    TS_FILE_DIR "${PROJECT_SOURCE_DIR}/i18n"
    MERGE_QT_TRANSLATIONS
    LUPDATE_OPTIONS -no-obsolete
)

target_link_libraries(app PRIVATE
    Qt::Gui
    Qt::Quick
    # Nothing calls QtSvg directly, but the deploy tool only ships the SVG icon plugin with it.
    Qt::Svg
    app_support
    app_ui
    app_uiplugin
    # Static QML modules are registered by linking their plugins.
    app_controlsplugin
    app_notify
    app_notifyplugin
    app_canvas # demo
    app_canvasplugin # demo
    app_sketchbook # demo
    app_sketchbookplugin # demo
)
app_target_defaults(app)
add_custom_target(run COMMAND app USES_TERMINAL VERBATIM)

app_add_qt_notices()

app_copy_notices(app)

# Qt's deploy tool makes the installed app self-contained. It runs last, after every file is in place.
install(TARGETS app
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
app_archive_symbols(app)

if(WIN32)
    # The deploy tool knows Qt's DLLs but not vcpkg's.
    install(FILES "$<TARGET_RUNTIME_DLLS:app>" DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
qt_generate_deploy_qml_app_script(
    TARGET app
    OUTPUT_SCRIPT _deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    # Qt's own strings are already merged into the app's translations (MERGE_QT_TRANSLATIONS).
    NO_TRANSLATIONS
    # Dependency symbols have no separate archive; preserve them during deployment.
    DEPLOY_TOOL_OPTIONS $<$<PLATFORM_ID:Darwin>:-no-strip>
)
install(SCRIPT "${_deploy_script}")
