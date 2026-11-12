<h1 align="center">Qt <s>Quick</s> Kickstart</h1>

<p align="center">So, you want to build a Qt Quick app. Where do you start, how do you structure it, and how do you get it onto someone else's machine?</p>

<p align="center">
  <img width="1086" alt="Sketchbook canvas with a triangle and a rectangle" src="https://github.com/user-attachments/assets/317e53ad-baaa-48ea-90f7-dcb4f6be9521" />
</p>

<p align="center">
  <a href="https://github.com/deeedob/qt-kickstart/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/deeedob/qt-kickstart/actions/workflows/ci.yml/badge.svg" /></a>
  <a href="https://github.com/deeedob/qt-kickstart/actions/workflows/docs.yml"><img alt="Guide" src="https://github.com/deeedob/qt-kickstart/actions/workflows/docs.yml/badge.svg" /></a>
  <a href="https://github.com/deeedob/qt-kickstart/actions/workflows/git-policy.yml"><img alt="Git policy" src="https://github.com/deeedob/qt-kickstart/actions/workflows/git-policy.yml/badge.svg" /></a>
  <a href="https://github.com/deeedob/qt-kickstart/actions/workflows/release.yml"><img alt="Release packages" src="https://github.com/deeedob/qt-kickstart/actions/workflows/release.yml/badge.svg" /></a>
</p>

<p align="center">
  <a href="https://deeedob.github.io/qt-kickstart/"><img src="docs/images/read-guide.svg" width="300" height="64" alt="Read the Qt Kickstart guide" /></a>
</p>

Qt Kickstart works through those questions with a hands-on guide and a template you can make your own. We cover the essentials of modern Qt Quick development: architecture and patterns, working with C++ and QML, and good development practices, from Git workflows to checking and testing your code. Then we take it through building, CI and packaging for different platforms.

Along the way, we explain the choices made, share practical tricks, and link to further reading whenever you want to dig deeper.

Qt 6 requires C++17 or newer. This template uses Qt 6.12, CMake 3.31, and C++23.

Throughout the guide, we use *Sketchbook*, a small app that recognises the shapes you draw. It gives us something concrete to build, test and package, while keeping the code small enough to follow.

The template includes:

- Typed QML modules with C++ state and a shared Controls style.
- Light and dark themes, translations, and persistent settings.
- Unit and QML tests, sanitizers, and one quality gate.
- CMake presets, dependency management, CI, and release packages.

## Start here

You need Qt 6.12, CMake 3.31, Ninja, and a C++23 compiler for this template.
Clone with its dependency submodule:

```sh
git clone --recurse-submodules https://github.com/deeedob/qt-kickstart.git
cd qt-kickstart
```

Choose one Qt source before building. For Qt installed in a standard system location:

```sh
cmake --preset dev -DAPP_USE_VCPKG_QT=OFF
cmake --workflow --preset dev
```

For a custom Qt installation, set `Qt6_ROOT` to the kit directory containing `lib/cmake/Qt6`.
For example, on macOS:

```sh
export Qt6_ROOT="$HOME/Qt/6.12.0/macos"
cmake --preset dev -DAPP_USE_VCPKG_QT=OFF
cmake --workflow --preset dev
```

To build Qt from source through vcpkg instead:

```sh
cmake --preset dev -DAPP_USE_VCPKG_QT=ON
cmake --workflow --preset dev
```

Building Qt through vcpkg takes longer on the first configure. The Qt source option stays
in the build directory's cache until you change it. Then start the app:

```sh
cmake --build --preset dev --target run
```

## Make it yours

Use this repository as a GitHub template, then run:

```sh
python3 tools/make-it-yours/make_it_yours.py
```

The script renames the application and project details. See [Customization](docs/customization.md)
to replace the application icon. The guide covers the reasoning,
from QML and C++ boundaries through tests, checks, and releases.

## License

[MIT-0](LICENSE) applies to this template. Bundled third-party files retain the licenses
listed in [REUSE.toml](REUSE.toml).
