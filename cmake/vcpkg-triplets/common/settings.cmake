# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# Settings shared by the overlay triplets one folder up, which shadow vcpkg's builtin ones.

include("${CMAKE_CURRENT_LIST_DIR}/../../MacOSDeploymentTarget.cmake")

# Dependencies are built once, in release only: halves build time and disk usage.
set(VCPKG_BUILD_TYPE release)
set(VCPKG_OSX_DEPLOYMENT_TARGET "${APP_MACOS_DEPLOYMENT_TARGET}")

# libdwarf (used by cpptrace) is LGPL-2.1: linked dynamically, so users can replace it.
if(PORT STREQUAL "libdwarf")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

# vcpkg's binary cache hashes only the triplet file itself; included files must be listed.
list(APPEND VCPKG_HASH_ADDITIONAL_FILES
    "${CMAKE_CURRENT_LIST_FILE}"
    "${CMAKE_CURRENT_LIST_DIR}/../../MacOSDeploymentTarget.cmake"
)
