<h1 align="center">Qt <s>Quick</s> Kickstart</h1>

<img width="2172" height="724" alt="Unbenannt" src="https://github.com/user-attachments/assets/317e53ad-baaa-48ea-90f7-dcb4f6be9521" />

<p align="center">
  <a href="https://deeedob.github.io/qt-kickstart/"><strong>Read the guide →</strong></a>
</p>

So, you want to build a Qt Quick app. Where do you start, how do you structure it, and how do you get it onto someone else's machine?

Qt Kickstart works through those questions with a hands-on guide and a template you can make your own. We cover the essentials of modern Qt Quick development: architecture and patterns, working with C++ and QML, and good development practices, from Git workflows to checking and testing your code. Then we take it through building, CI and packaging for different platforms.

Along the way, we explain the choices made, share practical tricks, and link to further reading whenever you want to dig deeper.

Built with Qt 6.12, C++23 and CMake.

Throughout the guide, we use *Sketchbook*, a small app that recognises the shapes you draw. It gives us something concrete to build, test and package, while keeping the code small enough to follow.

## Prerequisites

You'll need CMake 3.31, Ninja and a C++ compiler (Xcode 16, Visual Studio 2022 or GCC 14).
The supplied project builds as C++23 to show modern C++ alongside Qt. When making the template
your own, you can choose another standard and adapt the code to it.

For Qt, choose either:

- **Use an existing Qt 6.12 installation**, including a system package, or install it with the [Qt Online Installer](https://www.qt.io/download-qt-installer-oss).
- **Build Qt as part of the project** with vcpkg. It builds the subset of Qt modules the app needs, so you don't need a separate Qt installation.

On Windows, run the commands in a *Developer PowerShell for VS 2022*.

## Build and run

Clone the project:

```sh
git clone --recurse-submodules https://github.com/deeedob/qt-kickstart.git
cd qt-kickstart
```

### With an installed Qt

Before configuring, set `Qt6_ROOT` to a Qt kit prefix. CMake stores it in the build directory:

```sh
cmake --preset dev -DQt6_ROOT=/path/to/Qt/6.12.0/<kit>
```

Otherwise, CMake searches `CMAKE_PREFIX_PATH` and its normal system locations. Then build, test
and run:

```sh
cmake --workflow --preset dev
cmake --build --preset dev --target run
```

### Building Qt with vcpkg

Enable the Qt feature for the `dev` build directory, then build, test and run:

```sh
cmake --preset dev -DAPP_USE_VCPKG_QT=ON
cmake --workflow --preset dev
cmake --build --preset dev --target run
```

The first Qt build can take hours; later builds can use vcpkg's binary cache. Outside Windows,
install autotools first. Set `APP_USE_VCPKG_QT=OFF` to use an installed Qt again. Set the
option separately for the `asan` or `release` preset when needed.

## Documentation screenshots

Build the showcase utility and give it an existing output directory:

```sh
cmake --build --preset dev --target showcase
./build/dev/tools/showcase/showcase /path/to/output
```

It writes four PNG files, replacing files with the same names. Run it in a native Qt graphics
session: the software renderer cannot capture the canvas strokes. The showcase uses temporary
application data. Fonts, display scaling, timestamps and build details can differ between runs.

## Git hooks

After initializing a project from this template, enable its local Git policy before your
first team commit:

```sh
git config core.hooksPath .githooks
```

The hooks check commit messages, staged whitespace, and staged C++ and QML formatting. They
need `clang-format` for C++ changes and Qt's `qmlformat` for QML changes. The guide's
[Git chapter](docs/17-git.qmd) explains the policy and how to enforce it for pull requests.

## Make it yours

Click *Use this template* on GitHub, clone your repository, then run:

```sh
python3 tools/make_it_yours.py
```

The script walks you through naming your app and updating the project details. You can also remove the demo and guide.

## What's included

- **Build presets** for development, sanitizers and release packages.
- **Code checks and formatting** for C++, QML and licensing.
- **Compiler settings** for warnings, Qt strict mode and hardening.
- **Dependency management** through vcpkg.
- **Packaging and CI** for Linux, macOS and Windows.
- **Build information and third-party notices** included in your app.

The guide explains how these fit together and how to adapt them.

## Get involved

Have feedback, a question or an idea for improving Qt Kickstart? [Open an issue](https://github.com/deeedob/qt-kickstart/issues) and join the conversation. Contributions are welcome.

## License

[MIT-0](LICENSE): use it for anything, no attribution required. Bundled third-party files
keep their own licenses, listed in [REUSE.toml](REUSE.toml).
