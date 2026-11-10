# AGENTS.md

A cross-platform Qt Quick application: **Qt 6.12, C++23, CMake 3.31, vcpkg**. QML shows the
state; C++ holds it. Write current Qt 6.12 code: much Qt code online is Qt 5 and still
compiles, but it is not how this project works. For general Qt, QML and CMake guidance, use
Qt's agent skills and documentation server
(https://github.com/TheQtCompanyRnD/agent-skills). This file covers what is specific to this
project.

## Commands

Run everything through the presets; never call the compiler or invent CMake flags.

```sh
cmake --workflow --preset dev               # configure, build, run all tests (Debug)
cmake --build --preset dev                  # rebuild after a change
ctest --preset dev                          # tests only
cmake --build --preset dev --target check   # every linter: must pass before you are done
cmake --build --preset dev --target format  # format C++ and QML in place
cmake --build --preset dev --target run     # start the app
cmake --workflow --preset asan              # tests with AddressSanitizer and UBSan
cmake --workflow --preset release           # what CI builds, tests and packages
```

Qt comes from `QT_ROOT_DIR` (an installed Qt 6.12). If configure says Qt is missing, ask the
user for the path; do not install Qt. Everything under `build/` is generated and disposable.

A change is done when the `dev` workflow passes and `check` is clean. Warnings are errors.

## Layout

```
libs/<name>/          separable plain C++ library: include/<name>/ (public .h), src/, tests/
src/<feature>/        one folder and one static QML module per feature: C++ and QML together
src/style/            the Qt Quick Controls style (QML module AppStyle) and Theme tokens
src/app/              the executable: main.cpp creates and wires every long-lived object
tests/<feature>/      tests for src/, mirroring its folders
cmake/                build helpers: app_target_defaults(), app_add_notice(), checks
i18n/                 translations (.ts)
external/vcpkg/       vcpkg as a git submodule: never edit
```

## Architecture rules

- Layers: View (QML) -> ViewModel or Store (QObject) -> Core (plain C++) -> Backend (files,
  network). A layer calls the one below; lower layers answer with signals and never include
  or call upward.
- Dependencies follow `target_link_libraries`. Never add an include path or link to get
  around a missing dependency; if code needs something above it, it is in the wrong layer.
- `main()` is the composition root: it creates long-lived objects before the
  `QQmlApplicationEngine` and passes them to QML. No globals, no hidden singletons.
- Code that does not need Qt goes into `libs/` as plain C++ with its own tests.

## C++

- Namespaces: `app::<feature>` in `src/`, the library's name in `libs/` (`<name>::`). File-local
  helpers go into an unnamed namespace. No `using namespace` at file scope in headers.
- One class per file, named after it. `.h` for public headers of `libs/`, `.hpp` for the rest.
  Include Qt by module: `#include <QtCore/QObject>`.
- `QT_NO_KEYWORDS` is on: write `Q_EMIT`, `Q_SIGNALS`, `Q_SLOTS`.
- Qt's strict mode (6.12) and `QT_NO_CAST_FROM_ASCII` are on: no implicit `char *` to `QString`
  (write `u"..."_s` or `"..."_L1`), no `QUrl` from a `QString` (`QUrl::fromLocalFile()`), every
  `connect()` to a lambda takes a context object, `open()` results are checked, no `QPair`
  (`std::pair`), and each file includes the Qt headers it uses.
- Every `Q_PROPERTY` has `NOTIFY` (or `CONSTANT`) and `FINAL`. In `Q_PROPERTY`, signals and
  `Q_INVOKABLE`s, spell types fully qualified (`app::<feature>::Type *`).
- Ownership: parent/child for QObjects, values or `std::unique_ptr` for the rest. A `QObject *`
  returned from a `Q_INVOKABLE` belongs to QML unless it has a parent.
- Never touch a QObject from another thread. Hand work off with `QtConcurrent` and return
  results through a continuation with a context object, or a queued signal.
- Save files with `QSaveFile`. Log through a `Q_STATIC_LOGGING_CATEGORY` per module, named
  `app.<module>`.
- Interfaces only at real seams (network, files, clock), named after what they are
  (`Transport`), never with an `I` prefix.

## Connecting C++ and QML

- Register types with `QML_ELEMENT` in `qt_add_qml_module(... STATIC ...)`. Link the module's
  `<target>plugin` into the executable and into QML tests, or it does not register.
- Objects created in `main()` reach QML as typed `required property`s through
  `setInitialProperties()`, and their classes are `QML_UNCREATABLE`. Never use
  `setContextProperty()`.
- App-wide state only: `QML_SINGLETON` with the instance from `setExternalSingletonInstance()`.
- Collections the user sees are `QAbstractListModel`s; report changes with
  `beginInsertRows()`/`dataChanged()`, not model resets.
- Custom drawing: a `QQuickItem` with a scene graph `updatePaintNode()`, which only builds
  nodes and reuses them between frames.
- C++ never reaches into QML: no `findChild()`, no `invokeMethod()` on QML objects.

## QML

- Typed QML: `pragma ComponentBehavior: Bound`, typed properties (never `property var` when a
  type exists), typed function signatures (`function f(x: real): void`), qualified access
  (`root.model`, not a bare `model`). Enums are numbers in QML: annotate parameters as `int`.
- Bindings, not assignments: never overwrite a bound property from JavaScript; change its
  source.
- Components declare their inputs as `required property` and their outputs as signals.
- Views import `AppStyle`, not `QtQuick.Controls`. Colors, spacing and radii come from `Theme`,
  never from literals.
- Every user-visible string goes through `qsTr()`.
- Keep JavaScript to small expressions; logic belongs in C++.

## Tests

- C++: Qt Test, one `tst_<Class>.cpp` per class. QML: Qt Quick Test, `tst_<Component>.qml`.
  Register them in the folder's `CMakeLists.txt` with `app_add_qt_test()` or
  `app_add_quick_test()`.
- Write the failing test first for a bug fix or a feature.
- Deterministic: `QTRY_*` / `tryCompare()` instead of sleeps, `QTemporaryDir` instead of real
  paths. Tests run with `QT_QPA_PLATFORM=offscreen`.

## Adding things

- A feature: `src/<feature>/` with `qt_add_qml_module(app_<feature> URI App.<Feature> STATIC
  ...)` and `app_target_defaults(app_<feature>)`, `add_subdirectory()` in `src/CMakeLists.txt`,
  link `app_<feature>` and `app_<feature>plugin` in `src/app/CMakeLists.txt`, tests in
  `tests/<feature>/`.
- A dependency: add the port to `vcpkg.json`, `find_package(... CONFIG REQUIRED)` in the
  top-level `CMakeLists.txt`.
- A third-party file (icon, font): add its license to `REUSE.toml`, and ship it with
  `app_add_notice()`.
- New strings: `cmake --build --preset dev --target update_translations`, then fill in the
  `.ts` files.

## Boundaries

- **Always:** run the `dev` workflow and `check` before you report a change as done; follow the
  existing patterns of the folder you are in.
- **Ask first:** new dependencies, changes to `CMakePresets.json`, `cmake/`, CI workflows or
  the warning and lint settings, new top-level folders.
- **Never:** edit `external/vcpkg/` or anything in `build/`; silence a warning or lint instead
  of fixing it; use Qt 5 APIs (`qmlRegisterType`, context properties, `QRegExp`,
  `Q_FOREACH`, `qt5_*` CMake macros); commit secrets or machine-specific paths.
