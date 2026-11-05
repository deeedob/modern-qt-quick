# How-to guides

Short recipes for common changes. For the reasoning behind the structure, see
[architecture](architecture.md).

## Rename the template

1. In the top-level `CMakeLists.txt`, change the name in `project()`, `APP_DISPLAY_NAME` and
   `APP_BUNDLE_ID`.
2. Replace the QML prefix `App` and the C++ namespace `app`, for example with `Acme`:

   ```sh
   rg -l 'App\.|"App"|URI App\b|AppStyle|app::|namespace app\b' src tests \
       | xargs perl -pi -e 's/\bApp\./Acme./g; s/"App"/"Acme"/g; s/URI App\b/URI Acme/; s/\bAppStyle\b/AcmeStyle/g; s/\bapp::/acme::/g; s/namespace app\b/namespace acme/g'
   ```

3. Put your copyright and license into `LICENSE` and the first entry of `REUSE.toml`, then
   build and run `check`.

## Remove the demo

1. Delete `src/canvas/`, `src/sketchbook/`, `tests/canvas/`, `tests/sketchbook/` and
   `libs/shapes/`, and their `add_subdirectory()` lines.
2. In `src/app/CMakeLists.txt`, remove the `app_canvas*` and `app_sketchbook*` entries.
3. In `main.cpp`, remove the storage, the store and the view model; in `Main.qml`, remove the
   two required properties, their imports and the pages.
4. Remove `glaze` from `vcpkg.json` and the top-level `CMakeLists.txt` if nothing else uses it.

Keep `src/style`: it is the base for your own views.

## Add a separable library

1. Create `libs/<name>/` with `include/<name>/`, `src/` and `tests/`, and a `CMakeLists.txt`
   modeled on `libs/shapes/CMakeLists.txt`. Call `app_target_defaults(<target>)` so the
   library gets warnings, hardening and checks.
2. Add `add_subdirectory(libs/<name>)` to the top-level `CMakeLists.txt`, before `src`.
3. Link it from the feature that uses it: `target_link_libraries(app_<feature> PUBLIC <name>::<name>)`.

## Add a feature

1. Create `src/<feature>/` with its C++ types, marked `QML_ELEMENT` and `QML_UNCREATABLE`, its
   pages, and a `qt_add_qml_module(app_<feature> URI App.<Feature> STATIC ...)`. Link
   `app_style` privately if its QML imports `AppStyle`.
2. Add `add_subdirectory(<feature>)` to `src/CMakeLists.txt`, before `app`.
3. Link `app_<feature>` and `app_<feature>plugin` in `src/app/CMakeLists.txt`.
4. In `main.cpp`, construct the feature's object before the engine and add it to
   `setInitialProperties()`; in `Main.qml`, declare it as a required property and pass it to
   the page.
5. Add `tests/<feature>/` with Qt Test or Qt Quick Test, and `add_subdirectory(<feature>)` to
   `tests/CMakeLists.txt`.

## Add a dependency from vcpkg

1. Add the port to `dependencies` in `vcpkg.json`.
2. Add `find_package(<Package> CONFIG REQUIRED)` to the top-level `CMakeLists.txt`, and link
   the target where you need it. The port's usage text, printed during configure, names the
   targets.
3. Reconfigure. Its license ships in the package's `licenses/` folder automatically.

## Ship a license notice for an asset

Put the asset's upstream license file next to it, register it in that folder's
`CMakeLists.txt`, and add the asset to `REUSE.toml`:

```cmake
app_add_notice(lucide-icons icons/LICENSE)
```

The package then contains `licenses/lucide-icons.txt`.

## Add a language

1. Add the language code to `I18N_TRANSLATED_LANGUAGES` in the top-level `CMakeLists.txt`.
2. Generate the new `.ts` file and translate it, for example with Qt Linguist:

   ```sh
   cmake --build --preset dev --target update_translations
   linguist i18n/app_fr.ts
   ```

3. Rebuild. The `.qm` file is embedded automatically, together with Qt's own translations for
   that language, and `AppController.language = "fr"` switches to it at runtime.

## Debug with more logging

```sh
QT_LOGGING_RULES="app.*.debug=true" build/dev/src/app/QtAppTemplate.app/Contents/MacOS/QtAppTemplate
```

On Windows and Linux, the executable is `build/dev/src/app/QtAppTemplate`. The session logs
are in the folder that *About, Open log folder* opens.
