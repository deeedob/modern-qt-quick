# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# CPack turns the installed, deployed application into one distributable per platform:
#   cmake --workflow --preset release      (configure, build, test, package)
# THIRD_PARTY_NOTICES.txt collects the licenses of everything shipped inside the package.

include_guard(GLOBAL)
include(GNUInstallDirs)

function(_app_write_third_party_notices output)
    set(_text "Third-party software shipped with ${APP_DISPLAY_NAME}\n")
    string(APPEND _text "=====================================================\n\n")
    string(APPEND _text
        "Qt (https://www.qt.io) is used under the GNU Lesser General Public License v3.\n"
        "The Qt libraries are linked dynamically and can be replaced. The Qt source code is\n"
        "available at https://download.qt.io/official_releases/qt/ and on request.\n\n")
    foreach(_license IN ITEMS LGPL-3.0-only GPL-3.0-only)
        file(READ "${PROJECT_SOURCE_DIR}/packaging/licenses/${_license}.txt" _license_text)
        string(APPEND _text "----- Qt: ${_license} -----\n${_license_text}\n")
    endforeach()

    # Assets compiled into the application: name | copyright | license text in LICENSES/.
    foreach(_entry IN ITEMS
            "Inter font|Copyright 2020 The Inter Project Authors|OFL-1.1"
            "Lucide icons|Copyright (c) 2026 Lucide Icons and Contributors|ISC"
            "Feather icons, via Lucide|Copyright (c) 2013-present Cole Bemis|MIT")
        string(REPLACE "|" ";" _parts "${_entry}")
        list(GET _parts 0 _name)
        list(GET _parts 1 _copyright)
        list(GET _parts 2 _license)
        file(READ "${PROJECT_SOURCE_DIR}/LICENSES/${_license}.txt" _license_text)
        string(APPEND _text "----- ${_name} (${_license}) -----\n${_copyright}\n\n${_license_text}\n")
    endforeach()

    # vcpkg installs each port's license as share/<port>/copyright.
    file(GLOB _copyrights "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/*/copyright")
    foreach(_file IN LISTS _copyrights)
        cmake_path(GET _file PARENT_PATH _dir)
        cmake_path(GET _dir FILENAME _port)
        # Test framework and build helpers are not shipped.
        if(_port MATCHES "^(catch2|vcpkg-.*)$")
            continue()
        endif()
        file(READ "${_file}" _license_text)
        string(APPEND _text "----- ${_port} (vcpkg) -----\n${_license_text}\n")
    endforeach()
    file(WRITE "${output}" "${_text}")
endfunction()

set(_notices "${PROJECT_BINARY_DIR}/THIRD_PARTY_NOTICES.txt")
_app_write_third_party_notices("${_notices}")
if(APPLE)
    install(FILES "${_notices}" DESTINATION "${PROJECT_NAME}.app/Contents/Resources")
else()
    install(FILES "${_notices}" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()

get_property(_git_sha GLOBAL PROPERTY APP_GIT_SHA)
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "${APP_DISPLAY_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-${_git_sha}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
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
