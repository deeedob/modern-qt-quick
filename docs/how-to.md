<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# How-to guides

Short recipes for common changes. For the reasoning behind the structure, see
[architecture](architecture.md).

## Rename the template

1. In the top-level `CMakeLists.txt`, change the name in `project()`, `APP_DISPLAY_NAME` and
   `APP_BUNDLE_ID`.
2. Replace the QML prefix `App` and the C++ namespace `app`, for example with `Acme`:

   ```sh
   rg -l 'App\.|"App"|URI App|app::|namespace app' src tests cmake \
       | xargs perl -pi -e 's/\bApp\./Acme./g; s/"App"/"Acme"/g; s/URI App\b/URI Acme/; s/\bapp::/acme::/g; s/namespace app\b/namespace acme/g'
   git mv src/libs/core/include/app src/libs/core/include/acme
   rg -l 'app/core/' src tests | xargs perl -pi -e 's{app/core/}{acme/core/}g'
   ```

3. Put your copyright into `LICENSE`, `REUSE.toml` and the SPDX headers, then build and run
   `check`.

## Remove the demo features

The timer and the tasks are independent. To remove the timer:

1. Delete `src/backend/timer/`, `src/frontend/timer/` and `tests/backend/tst_TimerStore.cpp`,
   and `FocusTimer.hpp` plus `tests/libs/core/tst_FocusTimer.cpp` from core.
2. Remove the matching `add_subdirectory()` lines, `app_add_qt_test(tst_TimerStore ...)`, and
   the two `*_timer*` entries in `src/app/CMakeLists.txt`.
3. In `main.cpp`, remove the `TimerStore`; in `Main.qml`, remove `required property TimerStore`,
   its import, the tab and the page.

The tasks feature comes out the same way, including the `TaskRepository` implementation in
`platform/` if nothing else needs it. Keep `frontend/ui`: it is the base for your own views.

## Add a pure C++ library

1. Create `src/libs/<name>/` with a `CMakeLists.txt` modeled on `src/libs/core/CMakeLists.txt`.
2. Add `add_subdirectory(<name>)` to `src/libs/CMakeLists.txt`.
3. Call `app_target_defaults(<target>)` so it gets warnings, hardening and checks.
4. Add Catch2 tests under `tests/libs/<name>/`.

## Add a feature

1. Create `src/backend/<feature>/` with C++ types marked `QML_ELEMENT` and
   `QML_UNCREATABLE`, and a `qt_add_qml_module(... URI App.Backend.<Feature> STATIC ...)`
   that links `Qt::Qml`.
2. Create `src/frontend/<feature>/` with the views, a `required property` for each backend
   object, and `qt_add_qml_module(... URI App.Frontend.<Feature> STATIC ...)`.
3. Link both `*plugin` targets in `src/app/CMakeLists.txt`.
4. In `main.cpp`, construct the backend object before the engine and add it to
   `setInitialProperties()`; in `Main.qml`, declare it as a required property and pass it to
   the view.

## Add a dependency from vcpkg

1. Add the port to `dependencies` in `vcpkg.json`.
2. Add `find_package(<Package> CONFIG REQUIRED)` to the top-level `CMakeLists.txt`, and link
   the target where you need it. The port's usage text, printed during configure, names the
   targets.
3. Reconfigure. Its license appears in `THIRD_PARTY_NOTICES.txt` automatically.

## Add a language

1. Add the language code to `I18N_TRANSLATED_LANGUAGES` in the top-level `CMakeLists.txt`.
2. Generate the new `.ts` file and translate it, for example with Qt Linguist:

   ```sh
   cmake --build --preset dev --target update_translations
   linguist i18n/app_fr.ts
   ```

3. Rebuild. The `.qm` file is embedded automatically, and `AppController.language = "fr"`
   switches to it at runtime.

## Debug with more logging

```sh
QT_LOGGING_RULES="app.*.debug=true" build/dev/src/app/QtAppTemplate.app/Contents/MacOS/QtAppTemplate
```

On Windows and Linux, the executable is `build/dev/src/app/QtAppTemplate`. The session logs
are in the folder that *About, Open log folder* opens.
