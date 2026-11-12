# CPack turns the installed, deployed application into one distributable per platform:
#   cmake --workflow --preset release      (configure, build, test, package)
#
#   app_add_notice(<name> <file>)
#       Ships <file>, a third-party license or notice, as licenses/<name>.txt inside the
#       package. Call it next to the code or asset it covers. Licenses of vcpkg dependencies
#       are added automatically.

include_guard(GLOBAL)
include(GNUInstallDirs)

function(app_set_icon target icon)
    if(NOT APPLE AND NOT WIN32)
        return()
    endif()

    cmake_path(ABSOLUTE_PATH icon BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    if(CMAKE_CROSSCOMPILING)
        find_program(APP_ICON_GENERATOR NAMES app_icon_generator NO_CMAKE_FIND_ROOT_PATH
            DOC "Host executable for generating application icons")
        if(NOT APP_ICON_GENERATOR)
            message(FATAL_ERROR "Set APP_ICON_GENERATOR to a host-built app_icon_generator executable.")
        endif()
        set(generator "${APP_ICON_GENERATOR}")
    else()
        set(generator app_icon_generator)
    endif()

    if(APPLE)
        set(appIcon "${CMAKE_CURRENT_BINARY_DIR}/app.icns")
        add_custom_command(
            OUTPUT "${appIcon}"
            COMMAND "${generator}" "${icon}" "${CMAKE_CURRENT_BINARY_DIR}/app.iconset"
            COMMAND iconutil -c icns "${CMAKE_CURRENT_BINARY_DIR}/app.iconset" -o "${appIcon}"
            DEPENDS "${generator}" "${icon}"
            VERBATIM
        )
        target_sources(${target} PRIVATE "${appIcon}")
        set_source_files_properties("${appIcon}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${appIcon}")
        # macOS caches bundle icons until the bundle directory changes.
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E touch "$<TARGET_BUNDLE_DIR:${target}>"
            VERBATIM
        )
        set_target_properties(${target} PROPERTIES MACOSX_BUNDLE_ICON_FILE app.icns)
    else()
        set(appIcon "${CMAKE_CURRENT_BINARY_DIR}/app.ico")
        add_custom_command(
            OUTPUT "${appIcon}"
            COMMAND "${generator}" "${icon}" "${appIcon}"
            DEPENDS "${generator}" "${icon}"
            VERBATIM
        )
        file(CONFIGURE OUTPUT app.rc CONTENT "IDI_ICON1 ICON \"${appIcon}\"\n")
        set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/app.rc" PROPERTIES OBJECT_DEPENDS "${appIcon}")
        set_source_files_properties("${appIcon}" PROPERTIES HEADER_FILE_ONLY TRUE)
        target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc" "${appIcon}")
    endif()
endfunction()

if(APPLE)
    set(APP_LICENSES_DIR "${PROJECT_NAME}.app/Contents/Resources/licenses")
    set(APP_LICENSES_RELATIVE_TO_EXECUTABLE "../Resources/licenses")
else()
    set(APP_LICENSES_DIR "${CMAKE_INSTALL_DOCDIR}/licenses")
    set(APP_LICENSES_RELATIVE_TO_EXECUTABLE "../${APP_LICENSES_DIR}")
endif()

function(app_add_notice name file)
    cmake_path(ABSOLUTE_PATH file BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    install(FILES "${file}" DESTINATION "${APP_LICENSES_DIR}" RENAME "${name}.txt")
    set_property(GLOBAL APPEND PROPERTY APP_NOTICE_FILES "${name}|${file}")
endfunction()

function(app_copy_notices target)
    get_property(notices GLOBAL PROPERTY APP_NOTICE_FILES)
    set(directory "${CMAKE_CURRENT_BINARY_DIR}/licenses")
    set(commands COMMAND "${CMAKE_COMMAND}" -E make_directory "${directory}")
    foreach(notice IN LISTS notices)
        string(REPLACE "|" ";" parts "${notice}")
        list(GET parts 0 name)
        list(GET parts 1 file)
        list(APPEND commands COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${file}" "${directory}/${name}.txt")
    endforeach()
    add_custom_command(TARGET ${target} POST_BUILD ${commands}
        COMMENT "Copying license notices for the development build" VERBATIM)
    target_compile_definitions(${target} PRIVATE APP_DEV_LICENSES_DIRECTORY="${directory}")
endfunction()

# vcpkg installs each port's license as share/<port>/copyright.
file(GLOB _app_copyrights "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/*/copyright")
foreach(_file IN LISTS _app_copyrights)
    cmake_path(GET _file PARENT_PATH _dir)
    cmake_path(GET _dir FILENAME _port)
    # The test framework and vcpkg's build helpers are not shipped.
    if(NOT _port MATCHES "^(catch2|vcpkg-.*)$")
        app_add_notice(${_port} "${_file}")
    endif()
endforeach()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${APP_DISPLAY_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-${APP_GIT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_DMG_VOLUME_NAME "${APP_DISPLAY_NAME}")
if(APPLE)
    set(CPACK_GENERATOR DragNDrop)
elseif(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()
include(CPack)

function(app_archive_symbols target)
    set(symbolDirectory "${PROJECT_BINARY_DIR}/symbols/$<CONFIG>")
    if(APPLE)
        find_program(APP_DSYMUTIL dsymutil REQUIRED)
        if(NOT CMAKE_STRIP)
            message(FATAL_ERROR "Release symbol archival requires strip.")
        endif()
        set(installedBinary "${PROJECT_NAME}.app/Contents/MacOS/$<TARGET_FILE_NAME:${target}>")
        set(commands [[
            execute_process(COMMAND "@APP_DSYMUTIL@" "$<TARGET_FILE:@target@>"
                -o "@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.dSYM"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_STRIP@" -S "${installedBinary}"
                COMMAND_ERROR_IS_FATAL ANY)
        ]])
    elseif(MSVC)
        set(commands [[
            file(COPY "$<TARGET_PDB_FILE:@target@>" DESTINATION "@symbolDirectory@")
        ]])
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(NOT CMAKE_OBJCOPY OR NOT CMAKE_STRIP)
            message(FATAL_ERROR "Release symbol archival requires objcopy and strip.")
        endif()
        set(installedBinary "${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${target}>")
        set(commands [[
            execute_process(COMMAND "@CMAKE_OBJCOPY@" --only-keep-debug
                "$<TARGET_FILE:@target@>" "@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.debug"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_STRIP@" --strip-debug "${installedBinary}"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_OBJCOPY@"
                "--add-gnu-debuglink=@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.debug"
                "${installedBinary}" COMMAND_ERROR_IS_FATAL ANY)
        ]])
    else()
        set(commands [[
            message(FATAL_ERROR "Symbol archival supports macOS, MSVC and Linux toolchains.")
        ]])
    endif()
    string(CONFIGURE "${commands}" commands @ONLY)
    set(script [[
        if(CMAKE_INSTALL_CONFIG_NAME STREQUAL "RelWithDebInfo")
            set(installedBinary "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/@installedBinary@")
            file(MAKE_DIRECTORY "@symbolDirectory@")
            @commands@
            file(WRITE "@symbolDirectory@/build.txt"
                "@PROJECT_NAME@ @PROJECT_VERSION@\ncommit=@APP_GIT_SHA@\nplatform=@CMAKE_SYSTEM_NAME@\narchitecture=@CMAKE_SYSTEM_PROCESSOR@\nconfiguration=$<CONFIG>\n")
        endif()
    ]])
    string(CONFIGURE "${script}" script @ONLY)
    install(CODE "${script}")
endfunction()
