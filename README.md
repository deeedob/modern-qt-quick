<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# Modern Qt 6 App Template

A GitHub template for Qt Quick applications with Qt 6.12, C++23 and CMake. It covers the
whole path from source to installer: a layered architecture that the build enforces,
dependencies from vcpkg, strict warnings and sanitizers, a single `check` target for every
linter, tests for each layer, translations, bundled fonts and icons, crash logging, and
packaging per platform.

The included demo app, *Focus Tasks*, is deliberately small: a task list with fuzzy search
and a focus timer. Each part of it shows one pattern: MVVM, Flux, dependency injection
into QML, and a custom scene graph item.

## Quick start

You need Git, CMake 3.31 or newer, Ninja, a C++23 compiler (Xcode 16, Visual Studio 2022,
or GCC 14) and Qt 6.12 from the [Qt Online Installer](https://www.qt.io/download-qt-installer-oss)
or [aqtinstall](https://github.com/miurahr/aqtinstall).

```sh
git clone --recurse-submodules https://github.com/<you>/<your-app>.git
cd <your-app>
export QT_ROOT_DIR=~/Qt/6.12.0/macos        # Windows: $env:QT_ROOT_DIR="C:\Qt\6.12.0\msvc2022_64"
cmake --workflow --preset dev               # configure, build, run all tests
```

The first configure lets vcpkg build the non-Qt dependencies, which takes a few minutes.
On Windows, run the commands from a *Developer PowerShell for VS 2022*.

| Command | Result |
|---|---|
| `cmake --workflow --preset dev` | Debug build and all tests |
| `cmake --workflow --preset asan` | Debug build and tests with AddressSanitizer and UBSan |
| `cmake --workflow --preset release` | Release build, tests, and an installer in `build/release/` |
| `cmake --workflow --preset vcpkg-qt` | Same as release, but vcpkg builds Qt itself (takes hours) |
| `cmake --build --preset dev --target check` | Every linter: format, clangd-tidy, qmllint, REUSE |
| `cmake --build --preset dev --target format` | Format C++ and QML in place |

## Layout

```
src/
  libs/core/        pure C++23, no Qt: domain types, fuzzy search, timer reducer (public API in include/)
  platform/         Qt without QML: storage, logging, crash handler, build info
  backend/<f>/      QML modules of C++ types: view models, stores, item models (no Qt Quick)
  frontend/ui/      design system: theme, reusable components, fonts, icons, a C++ scene graph item
  frontend/<f>/     the views of each feature
  app/              the executable: main.cpp wires everything together
tests/              one folder per layer, with Catch2, Qt Test and Qt Quick Test
cmake/              build policy: warnings, hardening, sanitizers, checks, packaging
i18n/               translations (.ts)
packaging/          icons and license texts for the installers
3rdparty/vcpkg/     vcpkg, pinned as a shallow git submodule
```

Each layer may only use the layers above it in this list, and `target_link_libraries` enforces
that: a view model that includes a Qt Quick header does not compile.

## Make it yours

1. Click *Use this template* on GitHub, then clone with `--recurse-submodules`.
2. Set your name, display name and bundle id in the top-level `CMakeLists.txt`
   (`project()`, `APP_DISPLAY_NAME`, `APP_BUNDLE_ID`) and your copyright in `LICENSE` and `REUSE.toml`.
3. Optionally rename the `App` QML prefix and the `app` namespace, see
   [docs/how-to.md](docs/how-to.md#rename-the-template).
4. Delete the demo features you do not need, see [docs/how-to.md](docs/how-to.md#remove-the-demo-features).

## Documentation

- [Architecture](docs/architecture.md): the layers, why they exist, and how the build enforces them.
- [QML and C++](docs/qml-integration.md): dependency injection, MVVM, Flux, and custom items.
- [Build and dependencies](docs/build-and-dependencies.md): presets, vcpkg, and where Qt comes from.
- [Code quality](docs/quality.md): warnings, hardening, sanitizers, linters, and tests.
- [Packaging and licensing](docs/packaging-and-licensing.md): installers, SPDX, and LGPL obligations.
- [How-to guides](docs/how-to.md): add a library or feature, add a language, remove the demo.

## License

The template is licensed under [MIT-0](LICENSE): use it for anything, no attribution required.
Bundled third-party assets keep their own licenses, listed in [REUSE.toml](REUSE.toml).
