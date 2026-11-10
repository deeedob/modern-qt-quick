# Settings shared by the overlay triplets one folder up, which shadow vcpkg's builtin ones.

# Dependencies are built once, in release only: halves build time and disk usage.
set(VCPKG_BUILD_TYPE release)

# The oldest macOS comes from MACOSX_DEPLOYMENT_TARGET (set in CMakePresets.json), the same
# variable CMake reads for the app. Passing it through also adds it to the binary-cache hash.
set(VCPKG_OSX_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}")
list(APPEND VCPKG_ENV_PASSTHROUGH MACOSX_DEPLOYMENT_TARGET)

# LGPL libraries are linked dynamically, so users can replace them: libdwarf (used by
# cpptrace) and, with the qt-from-source preset, Qt itself.
if(PORT STREQUAL "libdwarf" OR PORT MATCHES "^qt")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

# vcpkg's binary cache hashes only the triplet file itself; included files must be listed.
list(APPEND VCPKG_HASH_ADDITIONAL_FILES "${CMAKE_CURRENT_LIST_FILE}")
