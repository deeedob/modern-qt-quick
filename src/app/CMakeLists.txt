# Logging, crash handling and build info, as a library so tests can link them.
qt_add_library(app_support STATIC
    Logging.cpp Logging.hpp
)
app_add_build_info(app_support)
target_include_directories(app_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_support
    PUBLIC Qt::Core Qt::Qml
    PRIVATE cpptrace::cpptrace
)
app_target_defaults(app_support)

if(CMAKE_CROSSCOMPILING)
    if(APPLE OR WIN32)
        find_program(APP_ICON_GENERATOR NAMES app_icon_generator NO_CMAKE_FIND_ROOT_PATH
            DOC "Host executable for generating application icons")
        if(NOT APP_ICON_GENERATOR)
            message(FATAL_ERROR "Set APP_ICON_GENERATOR to a host-built app_icon_generator executable.")
        endif()
        set(_icon_generator "${APP_ICON_GENERATOR}")
    endif()
else()
    set(_icon_generator app_icon_generator)
endif()

qt_add_executable(app MACOSX_BUNDLE WIN32 main.cpp)
if(APPLE)
    set(_app_icon "${CMAKE_CURRENT_BINARY_DIR}/app.icns")
    add_custom_command(
        OUTPUT "${_app_icon}"
        COMMAND "${_icon_generator}" "${CMAKE_CURRENT_SOURCE_DIR}/icons/app.svg" "${CMAKE_CURRENT_BINARY_DIR}/app.iconset"
        COMMAND iconutil -c icns "${CMAKE_CURRENT_BINARY_DIR}/app.iconset" -o "${_app_icon}"
        DEPENDS "${_icon_generator}" icons/app.svg
        VERBATIM
    )
    target_sources(app PRIVATE "${_app_icon}")
    set_source_files_properties("${_app_icon}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_property(TARGET app APPEND PROPERTY LINK_DEPENDS "${_app_icon}")
    # macOS caches bundle icons until the bundle directory changes.
    add_custom_command(TARGET app POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E touch "$<TARGET_BUNDLE_DIR:app>"
        VERBATIM
    )
elseif(WIN32)
    # Windows takes the executable's icon from a resource script.
    set(_app_icon "${CMAKE_CURRENT_BINARY_DIR}/app.ico")
    add_custom_command(
        OUTPUT "${_app_icon}"
        COMMAND "${_icon_generator}" "${CMAKE_CURRENT_SOURCE_DIR}/icons/app.svg" "${_app_icon}"
        DEPENDS "${_icon_generator}" icons/app.svg
        VERBATIM
    )
    file(CONFIGURE OUTPUT app.rc CONTENT "IDI_ICON1 ICON \"${_app_icon}\"\n")
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/app.rc" PROPERTIES OBJECT_DEPENDS "${_app_icon}")
    set_source_files_properties("${_app_icon}" PROPERTIES HEADER_FILE_ONLY TRUE)
    target_sources(app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc" "${_app_icon}")
endif()
set_target_properties(app PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_ICON_FILE app.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}"
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)
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
        icons/app.svg
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

# Qt is linked dynamically under the LGPLv3, which asks for its license texts in the package.
# A Qt built by vcpkg brings them in its own copyright file, which Packaging.cmake ships.
app_add_notice(qt licenses/NOTICE.txt)
cmake_path(IS_PREFIX VCPKG_INSTALLED_DIR "${Qt6_DIR}" NORMALIZE _qt_from_vcpkg)
if(NOT _qt_from_vcpkg)
    app_add_notice(qt-lgpl-3.0 licenses/LGPL-3.0-only.txt)
    app_add_notice(qt-gpl-3.0 licenses/GPL-3.0-only.txt)
endif()

app_copy_notices(app)

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
    # Qt's own strings are already merged into the app's translations (MERGE_QT_TRANSLATIONS).
    NO_TRANSLATIONS
    # Keep symbols so crash traces in the log name functions, not just addresses.
    DEPLOY_TOOL_OPTIONS $<$<PLATFORM_ID:Darwin>:-no-strip>
)
install(SCRIPT "${_deploy_script}")
