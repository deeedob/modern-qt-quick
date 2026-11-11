# CPack turns the installed, deployed application into one distributable per platform:
#   cmake --workflow --preset release      (configure, build, test, package)
#
#   app_add_notice(<name> <file>)
#       Ships <file>, a third-party license or notice, as licenses/<name>.txt inside the
#       package. Call it next to the code or asset it covers. Licenses of vcpkg dependencies
#       are added automatically.

include_guard(GLOBAL)
include(GNUInstallDirs)

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
