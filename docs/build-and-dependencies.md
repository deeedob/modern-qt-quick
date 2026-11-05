# Build and dependencies

This document explains the build setup: presets, where dependencies come from, and the
choices behind them.

## Presets are the interface

`CMakePresets.json` holds every supported configuration, so nobody has to remember cache
variables. A *workflow* preset runs configure, build, test and package in one command:

```sh
cmake --workflow --preset dev
```

| Preset | Build type | Adds |
|---|---|---|
| `dev` | Debug | |
| `asan` | Debug | `APP_SANITIZERS=address;undefined` |
| `release` | RelWithDebInfo | a package step (CPack) |
| `vcpkg-qt` | RelWithDebInfo | Qt built by vcpkg |

All presets use Ninja, export `compile_commands.json` for clangd, and treat warnings as errors.
Machine-specific settings, such as a local Qt path or a compiler launcher, belong in a
`CMakeUserPresets.json`, which git ignores:

```json
{
  "version": 10,
  "configurePresets": [
    {
      "name": "my-dev",
      "inherits": "dev",
      "environment": { "QT_ROOT_DIR": "$env{HOME}/Qt/6.12.0/macos" },
      "cacheVariables": { "CMAKE_CXX_COMPILER_LAUNCHER": "ccache" }
    }
  ]
}
```

## Where Qt comes from

Qt is large, and building it takes hours. The template therefore expects an installed Qt by
default, and the presets pass `$env{QT_ROOT_DIR}` to `CMAKE_PREFIX_PATH`. Both the Qt Online
Installer and aqtinstall produce such an installation, and GitHub's `install-qt-action` sets
`QT_ROOT_DIR` automatically.

The `vcpkg-qt` preset builds Qt from source instead, through the `qt` feature in `vcpkg.json`.
That makes the build independent of any installed Qt, at the cost of a long first build.
vcpkg's ports may trail the latest Qt release by a few weeks.

## vcpkg for everything else

vcpkg builds the C++ dependencies from source on first configure: glaze (JSON through
compile-time reflection), cpptrace (stack traces) and Catch2 (tests). It runs in *manifest mode*: `vcpkg.json` lists the dependencies, and the vcpkg
toolchain file installs them into the build directory during `cmake --preset`.

vcpkg itself is a git submodule in `3rdparty/vcpkg`, cloned shallowly (`shallow = true` in
`.gitmodules`). The submodule commit pins every port version, so no `builtin-baseline` is
needed, and the toolchain bootstraps the `vcpkg` executable on first use. Update all
dependencies by moving the submodule:

```sh
git -C 3rdparty/vcpkg fetch --depth 1 origin master
git -C 3rdparty/vcpkg checkout FETCH_HEAD
git add 3rdparty/vcpkg
```

Built packages land in vcpkg's binary cache (`~/.cache/vcpkg/archives` on macOS and Linux,
`%LOCALAPPDATA%\vcpkg\archives` on Windows), so other projects and clean builds reuse them.

### Release-only dependencies

`cmake/vcpkg-triplets/` contains overlay triplets that shadow vcpkg's built-in ones
(`arm64-osx`, `x64-linux`, ...). Each includes the built-in triplet and
`common/settings.cmake`, which sets `VCPKG_BUILD_TYPE release`: that halves the build time
and disk use, and a Debug build of the app links the release libraries. The shared settings
also build libdwarf as a shared library (see [packaging and licensing](packaging-and-licensing.md)).

The oldest supported macOS is set once, as `MACOSX_DEPLOYMENT_TARGET` in the `environment` of
the base preset. That is Apple's standard variable: CMake initializes
`CMAKE_OSX_DEPLOYMENT_TARGET` from it, and the triplets read it for the dependencies, so the app
and its libraries always agree. The triplets also pass it through `VCPKG_ENV_PASSTHROUGH`, which
adds its value to vcpkg's binary-cache hash: changing it rebuilds the dependencies instead of
reusing ones built for another macOS. Override it in your `CMakeUserPresets.json` if you need
an older or newer minimum.

vcpkg's binary cache hashes only the triplet file itself, so `common/settings.cmake` adds itself
to `VCPKG_HASH_ADDITIONAL_FILES`.

Windows keeps vcpkg's default triplet, which builds debug and release. MSVC's debug runtime
is not compatible with release libraries, so a Debug build there needs debug dependencies.

## Project-wide settings

The top-level `CMakeLists.txt` sets the few things that apply to everything: C++23 without
compiler extensions, the application name and bundle id. All
other settings are attached to targets. In particular, warnings, hardening and sanitizers come
from one function, `app_target_defaults()`, described in [code quality](quality.md).

`cmake/BuildInfo.cmake` generates a single source file with the version, git commit, build
type, compiler and target system, so only that file recompiles when they change. CMake re-runs
after every commit or checkout, which keeps the commit hash current; the `-dirty` marker
reflects the working tree at the last configure.
