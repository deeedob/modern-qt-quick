<h1 align="center">Qt <s>Quick</s> Kickstart</h1>

<img width="2172" height="724" alt="Sketchbook canvas with a triangle and a rectangle" src="https://github.com/user-attachments/assets/317e53ad-baaa-48ea-90f7-dcb4f6be9521" />

<p align="center">
  <a href="https://deeedob.github.io/qt-kickstart/"><strong>Read the guide →</strong></a>
</p>

[![CI](https://github.com/deeedob/qt-kickstart/actions/workflows/ci.yml/badge.svg)](https://github.com/deeedob/qt-kickstart/actions/workflows/ci.yml)
[![Guide](https://github.com/deeedob/qt-kickstart/actions/workflows/docs.yml/badge.svg)](https://github.com/deeedob/qt-kickstart/actions/workflows/docs.yml)
[![Git policy](https://github.com/deeedob/qt-kickstart/actions/workflows/git-policy.yml/badge.svg)](https://github.com/deeedob/qt-kickstart/actions/workflows/git-policy.yml)
[![Release packages](https://github.com/deeedob/qt-kickstart/actions/workflows/release.yml/badge.svg)](https://github.com/deeedob/qt-kickstart/actions/workflows/release.yml)

So, you want to build a Qt Quick app. Where do you start, how do you structure it, and how do you get it onto someone else's machine?

Qt Kickstart works through those questions with a hands-on guide and a template you can make your own. We cover the essentials of modern Qt Quick development: architecture and patterns, working with C++ and QML, and good development practices, from Git workflows to checking and testing your code. Then we take it through building, CI and packaging for different platforms.

Along the way, we explain the choices made, share practical tricks, and link to further reading whenever you want to dig deeper.

Built with Qt 6.12, C++23 and CMake.

Throughout the guide, we use *Sketchbook*, a small app that recognises the shapes you draw. It gives us something concrete to build, test and package, while keeping the code small enough to follow.

[![Read the Qt Kickstart guide](https://img.shields.io/badge/Read_the_guide-Qt_Kickstart-5B5BD6?style=for-the-badge)](https://deeedob.github.io/qt-kickstart/)

## Start here

You need Qt 6.12, CMake 3.31, Ninja, and a C++23 compiler. Point `Qt6_ROOT` at the Qt
installation, then clone and run the development workflow:

```sh
git clone --recurse-submodules https://github.com/deeedob/qt-kickstart.git
cd qt-kickstart
cmake --workflow --preset dev
cmake --build --preset dev --target run
```

## Make it yours

Use this repository as a GitHub template, then run:

```sh
python3 tools/make-it-yours/make_it_yours.py
```

The script renames the application and project details. The guide covers the reasoning,
from QML and C++ boundaries through tests, checks, and releases.

## License

[MIT-0](LICENSE) applies to this template. Bundled third-party files retain the licenses
listed in [REUSE.toml](REUSE.toml).
