<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# Packaging and licensing

This document explains how the template produces installers, and how it keeps track of the
licenses of everything it ships.

## From build to installer

```sh
cmake --workflow --preset release
```

configures, builds, tests, and then runs CPack, which leaves the package in
`build/release/`. The package file name contains the version, git commit, system and
architecture, for example `QtAppTemplate-0.1.0-1a2b3c4d5e6f-Darwin-arm64.dmg`.

Packaging happens in two steps:

1. `cmake --install` installs the executable and runs the script from
   `qt_generate_deploy_qml_app_script()`. That script calls Qt's deployment tool
   (`macdeployqt` or `windeployqt`; on Linux, CMake's own dependency resolution), which copies
   the Qt libraries, plugins and QML modules the app needs next to it. On Windows, the
   vcpkg DLLs are installed alongside.
2. CPack archives the installed tree: a drag-and-drop DMG on macOS, a ZIP on Windows, a
   tarball on Linux. Change `CPACK_GENERATOR` in `cmake/Packaging.cmake` for NSIS, WiX or
   DEB installers.

The bundle id, display name, version and icons come from the top-level `CMakeLists.txt` and
`packaging/`. Signing and notarization depend on your certificates and are not part of the
template.

## SPDX headers and REUSE

Every file states its license in a machine-readable header:

```cpp
// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0
```

Files that cannot carry a comment, such as JSON, translations, fonts and icons, are covered
by `REUSE.toml`. The license texts themselves live in `LICENSES/`, one file per license
identifier. The [REUSE](https://reuse.software) tool verifies that nothing is missing:

```sh
cmake --build --preset dev --target check_reuse
```

The template's own code is MIT-0, which lets you relicense your application freely. The
bundled assets keep their licenses: the Inter font (OFL-1.1) and the Lucide icons (ISC, and
MIT for icons derived from Feather).

## Notices for shipped binaries

`LICENSES/` covers the repository. A distributed application must also carry the notices of
everything compiled into it. `cmake/Packaging.cmake` therefore generates
`THIRD_PARTY_NOTICES.txt` at configure time and installs it with the app. It contains:

- the Qt notice, with the full LGPLv3 and GPLv3 texts from `packaging/licenses/`;
- the font and icon licenses, with their copyright lines;
- the license of every vcpkg port the app links, read from vcpkg's
  `share/<port>/copyright` files. Catch2 and vcpkg's build helpers are left out, since they
  are not shipped.

A new vcpkg dependency appears in the notices automatically.

## Using Qt under the LGPL

Qt's open-source license is LGPLv3. For a distributed application, that means:

- link Qt dynamically, so users can replace it (the template's default);
- ship the license texts and a notice (`THIRD_PARTY_NOTICES.txt`, and the About dialog);
- provide Qt's source code, or a written offer for it;
- allow users to install a modified Qt, which rules out locked-down devices;
- avoid the modules available only under the GPL, such as Qt Charts.

cpptrace depends on libdwarf, which is LGPL-2.1. The overlay triplets in
`cmake/vcpkg-triplets/` build libdwarf as a shared library, so the same "replaceable library"
reasoning applies to it.

This section summarizes obligations; it is not legal advice.
