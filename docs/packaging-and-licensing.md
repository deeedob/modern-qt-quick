# Packaging and licensing

This document explains how the template produces packages, and how it keeps track of the
licenses of everything it contains and ships.

## From build to package

```sh
cmake --workflow --preset release
```

configures, builds, tests, and then runs CPack, which leaves the package in
`build/release/`. The package file name contains the version, git commit, system and
architecture, for example `QtAppTemplate-0.1.0-1a2b3c4d5e6f-Darwin-arm64.dmg`.

Packaging happens in two steps:

1. `cmake --install` installs the executable, the license notices, and then runs the script
   from `qt_generate_deploy_qml_app_script()`. That script calls Qt's deployment tool
   (`macdeployqt` or `windeployqt`; on Linux, CMake's own dependency resolution), which copies
   the Qt libraries, plugins and QML modules the app needs next to it, and signs the macOS
   bundle ad hoc. It runs last, so the signature covers every installed file. On Windows, the
   vcpkg DLLs are installed alongside.
2. CPack archives the installed tree: a drag-and-drop DMG on macOS, a ZIP on Windows, a
   tarball on Linux. Change `CPACK_GENERATOR` in `cmake/Packaging.cmake` for NSIS, WiX or
   DEB installers.

The bundle id, display name and version come from the top-level `CMakeLists.txt`, the icons
from `src/app/`. Signing with a certificate and notarization depend on your accounts and are
not part of the template.

## The repository's licenses: REUSE

`REUSE.toml` declares the license of every file in the repository, in one place. Its first
entry covers everything; later entries override it for third-party files:

```toml
[[annotations]]
path = "**"
SPDX-FileCopyrightText = "2026 Dennis Oberst"
SPDX-License-Identifier = "MIT-0"

[[annotations]]
path = ["src/style/fonts/InterVariable.ttf", "src/style/fonts/LICENSE.txt"]
SPDX-FileCopyrightText = "2016 The Inter Project Authors"
SPDX-License-Identifier = "OFL-1.1"
```

When several entries match a path, the last one wins. Source files therefore carry no license
header, and a project built on the template changes its license by editing `LICENSE` and the
first entry. `LICENSES/` holds one generic text per license identifier, as the
[REUSE specification](https://reuse.software) requires; the copyright holders are in
`REUSE.toml`, which is why those texts keep their placeholders. The tool verifies that nothing
is missing:

```sh
cmake --build --preset dev --target check_reuse
```

## Shipped licenses: one file per component

A distributed application must carry the license of everything inside it. The package
contains a `licenses/` folder, with one file per component, each the upstream text unchanged:

| File | From |
|---|---|
| `qt.txt`, `qt-lgpl-3.0.txt`, `qt-gpl-3.0.txt` | `3rdparty/qt/`, registered in `src/app/CMakeLists.txt` |
| `inter-font.txt`, `lucide-icons.txt` | the upstream license files next to the assets in `src/style/` |
| one file per vcpkg port, such as `glaze.txt` and `cpptrace.txt` | vcpkg's `share/<port>/copyright`, collected automatically |

On macOS the folder is inside the bundle, in `Contents/Resources/licenses/`; elsewhere it is
in `share/doc/<project>/licenses/`. The About dialog tells users where to find it.

`app_add_notice(<name> <file>)` in `cmake/Packaging.cmake` registers one file. Call it next to
the code or asset the license covers, so adding or deleting an asset changes one folder. A new
vcpkg dependency needs no call; Catch2 and vcpkg's build helpers are left out, because they are
not shipped.

vcpkg also writes an SPDX software bill of materials for every port
(`share/<port>/vcpkg.spdx.json`), and `vcpkg license-report` lists the declared licenses of
all installed ports. Both are a starting point when a customer or regulation asks for an SBOM.

## Using Qt under the LGPL

Qt's open-source license is LGPLv3. For a distributed application, that means:

- link Qt dynamically, so users can replace it (the template's default);
- ship the license texts and a notice (`licenses/`, and the About dialog);
- provide Qt's source code, or a written offer for it (`3rdparty/qt/NOTICE.txt`);
- allow users to install a modified Qt, which rules out locked-down devices;
- avoid the modules available only under the GPL, such as Qt Charts.

cpptrace depends on libdwarf, which is LGPL-2.1. The overlay triplets in
`cmake/vcpkg-triplets/` build libdwarf as a shared library, so the same "replaceable library"
reasoning applies to it.

This section summarizes obligations; it is not legal advice.
