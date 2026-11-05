# Modern Qt 6 App Template

A GitHub template for Qt Quick applications with Qt 6.12, C++23 and CMake. It covers the
whole path from source to installer: a Pitchfork layout, dependencies from vcpkg, strict
warnings and sanitizers, one `check` target for every linter, tests, translations, a custom
Qt Quick Controls style, crash logging, and packages per platform with their license notices.

The demo app, *Sketchbook*, is small on purpose. You draw on a canvas, and circles,
rectangles and triangles snap into clean shapes. Every part of it exists to show one idea:

| Part | Shows |
|---|---|
| `libs/shapes` | Plain C++ that knows nothing about the app: the $1 gesture recognizer, with its own tests and benchmark |
| `src/canvas` | Flux: actions, a pure reducer, and a history that makes undo and redo trivial; a custom scene graph item |
| `src/sketchbook` | MVVM: a view model with an item model; saving on a worker thread with glaze |
| `src/style` | A Qt Quick Controls style with design tokens and light and dark themes |
| `src/app` | The composition root, logging, crash handling and build info |

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
| `cmake --workflow --preset release` | Release build, tests, and a package in `build/release/` |
| `cmake --workflow --preset vcpkg-qt` | Same as release, but vcpkg builds Qt itself (takes hours) |
| `cmake --build --preset dev --target check` | Every linter: format, clangd-tidy, qmllint, REUSE |
| `cmake --build --preset dev --target format` | Format C++ and QML in place |

## Layout

```
libs/shapes/        a separable library: plain C++, own include/, src/ and tests/
src/
  style/            the design system: a Qt Quick Controls style, fonts, icons
  canvas/           drawing (Flux) and the custom scene graph item
  sketchbook/       the list of sketches (MVVM) and their files
  app/              the executable: main.cpp wires everything together
tests/              tests for src/, mirroring its folders
cmake/              build policy: warnings, hardening, sanitizers, checks, packaging
i18n/               translations (.ts)
docs/               the guide, a Quarto book
3rdparty/           vcpkg as a git submodule, and Qt's license texts
```

## Make it yours

1. Click *Use this template* on GitHub, then clone with `--recurse-submodules`.
2. Set your name, display name and bundle id in the top-level `CMakeLists.txt`
   (`project()`, `APP_DISPLAY_NAME`, `APP_BUNDLE_ID`), and your copyright and license in
   `LICENSE` and the first entry of `REUSE.toml`.
3. Optionally rename the `App` QML prefix and the `app` namespace, see
   [Rename the template](docs/template.qmd#rename).
4. Replace the demo features, see [Remove the demo](docs/template.qmd#remove-demo).

## Documentation

`docs/` holds a guide to the concepts behind the template: the shape of a Qt Quick app, the
build, QML and C++, testing, and shipping. It is a [Quarto](https://quarto.org) book:

```sh
quarto preview docs      # opens the guide in your browser
quarto render docs       # writes the website and the PDF to docs/_book/
```

A rendered copy is in [docs/modern-qt-quick-applications.pdf](docs/modern-qt-quick-applications.pdf).

Recipes for common changes, such as adding a feature or a language, are in its appendix,
[Working with the template](docs/template.qmd).

## License

The template is licensed under [MIT-0](LICENSE): use it for anything, no attribution required.
Bundled third-party assets keep their own licenses, listed in [REUSE.toml](REUSE.toml).
