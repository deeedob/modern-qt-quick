# Logging, crash handling and build info, as a library so tests can link them.
qt_add_library(app_support STATIC
    Logging.cpp Logging.hpp
)
app_add_build_info(app_support)
target_include_directories(app_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_support
    PUBLIC Qt::Core
    PRIVATE cpptrace::cpptrace
)
app_target_defaults(app_support)

qt_add_executable(app MACOSX_BUNDLE WIN32 main.cpp)
if(APPLE)
    target_sources(app PRIVATE app.icns)
    set_source_files_properties(app.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32)
    # Windows takes the executable's icon from a resource script.
    file(CONFIGURE OUTPUT app.rc CONTENT "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/app.ico\"\n")
    target_sources(app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
endif()
set_target_properties(app PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_ICON_FILE app.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}"
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

# The executable is a QML module too (URI App), holding the window and the app-wide controller.
# The About dialog shows app.svg, the same artwork as the application icon.
qt_add_qml_module(app
    URI App
    QML_FILES
        AboutDialog.qml
        Main.qml
    SOURCES
        AppController.cpp AppController.hpp
    RESOURCES
        app.svg
)

target_link_libraries(app PRIVATE
    Qt::Gui
    Qt::Quick
    # Nothing calls QtSvg directly, but the deploy tool only ships the SVG icon plugin with it.
    Qt::Svg
    app_support
    # Static QML modules are registered by linking their plugins.
    app_styleplugin
    app_canvas # demo
    app_canvasplugin # demo
    app_sketchbook # demo
    app_sketchbookplugin # demo
)
app_target_defaults(app)
add_custom_target(run COMMAND app USES_TERMINAL VERBATIM)

qt_add_translations(app
    TS_FILE_BASE app
    TS_FILE_DIR "${PROJECT_SOURCE_DIR}/i18n"
    # Qt's own strings, such as the OK and Cancel buttons of a dialog, translated as well.
    MERGE_QT_TRANSLATIONS
    # Drop strings that no longer exist in the sources instead of keeping them as "vanished".
    LUPDATE_OPTIONS -no-obsolete
)

# Qt is linked dynamically under the LGPLv3, which asks for its license texts in the package.
# A Qt built by vcpkg brings them in its own copyright file, which Packaging.cmake ships.
app_add_notice(qt licenses/NOTICE.txt)
cmake_path(IS_PREFIX VCPKG_INSTALLED_DIR "${Qt6_DIR}" NORMALIZE _qt_from_vcpkg)
if(NOT _qt_from_vcpkg)
    app_add_notice(qt-lgpl-3.0 licenses/LGPL-3.0-only.txt)
    app_add_notice(qt-gpl-3.0 licenses/GPL-3.0-only.txt)
endif()

# Qt's deploy tool makes the installed app self-contained. It runs last, after every file is in place.
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
