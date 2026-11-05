<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# Code quality

This document explains how the template keeps code strict: compiler warnings, hardening,
sanitizers, linters and tests, each defined in exactly one place.

## One definition per concern

| Concern | Defined in | Used by |
|---|---|---|
| Compiler warnings | `cmake/AppTargetDefaults.cmake` | the build, and clangd through `compile_commands.json` |
| Warnings as errors | `CMAKE_COMPILE_WARNING_AS_ERROR` in `CMakePresets.json` | the build; `.clangd` removes `-Werror` for the editor |
| clang-tidy checks | `.clang-tidy` | clangd, `check_clangd`, plain clang-tidy |
| C++ formatting | `.clang-format` | the editor, `format`, `check_format` |
| QML formatting | `.qmlformat.ini` | qmlformat, `format` |
| QML lint severity | `.qmllint.ini` (any warning fails) | qmllint, `all_qmllint`, qmlls |
| Licensing | SPDX headers and `REUSE.toml` | `check_reuse` |

`.clangd` lists no warning flags of its own: clangd reads the real compile commands, so the
editor shows exactly what the compiler reports.

## Compiler warnings

Every first-party target calls `app_target_defaults(<target>)`. It applies a strict warning
set (`-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast`, and
more; `/W4` and selected extra warnings on MSVC). The presets turn warnings into errors. To
configure a build that tolerates warnings for a while:

```sh
cmake --preset dev --compile-no-warning-as-error
```

The flags go on each hand-written source file, not on the whole target. Qt compiles
generated code (moc output, QML type registrations, QML compiled to C++) into the same
targets, and that code is not ours to fix. Third-party headers arrive through imported
targets as system includes, so their warnings never show up.

## Hardening and sanitizers

`APP_ENABLE_HARDENING` (on by default) adds checks that cost little at runtime:

- standard library assertions (`_LIBCPP_HARDENING_MODE` for libc++, `_GLIBCXX_ASSERTIONS`
  for libstdc++), which turn out-of-bounds `operator[]` into a clean abort;
- `-fstack-protector-strong`, and `-fcf-protection` and `-fstack-clash-protection` where the
  compiler supports them;
- `_FORTIFY_SOURCE=3` outside Debug builds;
- `/sdl /guard:cf` on MSVC.

`APP_SANITIZERS` takes a list of sanitizers; the `asan` preset sets `address;undefined`.
Incompatible combinations, such as thread with address, stop the configure step with an
explanation. Qt itself is not instrumented, so the test preset disables AddressSanitizer's
container-overflow check, which reports false positives across instrumented and
uninstrumented code.

Some Apple clang releases hang in AddressSanitizer's startup on recent macOS versions,
before any project code runs. If `asan` tests never finish, check with a two-line program;
a Homebrew LLVM (`CMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++` in a user preset)
works as a replacement.

## The check target

```sh
cmake --build --preset dev --target check
```

runs every linter whose tool is installed:

| Target | Tool | Install |
|---|---|---|
| `check_format` | clang-format | LLVM |
| `check_clangd` | [clangd-tidy](https://github.com/lljbash/clangd-tidy) | `pipx install clangd-tidy`, plus clangd from LLVM |
| `all_qmllint` | qmllint | ships with Qt |
| `check_reuse` | [reuse](https://reuse.software) | `pipx install "reuse[charset-normalizer]"` |

A missing tool skips its target and prints the install command during configure.
`check_clangd` runs clangd on every first-party file, exactly as the editor would: compiler
diagnostics, clang-tidy and unused includes, in parallel and much faster than plain
clang-tidy. Run it from time to time to catch problems in files nobody has opened recently.

clang-tidy starts from every check and removes vendor-specific groups and checks that
conflict with Qt idioms (moc, integer model roles, logging macros) or with the formatting
style. Identifier naming is left to each project.

## Tests

Each layer is tested with the lightest tool that can test it:

| Layer | Framework | What the demo tests |
|---|---|---|
| `libs` | Catch2 | fuzzy search, and the timer reducer at compile time |
| `platform` | Qt Test | the JSON repository, log files and their rotation |
| `backend` | Qt Test, without Qt Quick | view model and store behavior, with `QAbstractItemModelTester` and `QSignalSpy` |
| `frontend` | Qt Quick Test | component APIs, clicks, and the ring's pixels |
| `app` | CTest | `--version` starts the real executable |

The timer reducer is `constexpr`, so `tests/libs/core/tst_FocusTimer.cpp` checks it with
`STATIC_REQUIRE`: a broken reducer fails the build. The same file builds a second time with
`CATCH_CONFIG_RUNTIME_STATIC_REQUIRE` as `tst_core_relaxed`, which runs the checks at runtime
where a debugger can step through them.

The test presets set `QT_QPA_PLATFORM=offscreen`, so no windows appear. Qt Quick then uses
its software renderer, which cannot draw custom scene graph geometry; tests that compare
pixels skip themselves there. Run them on the GPU with:

```sh
QT_QPA_PLATFORM= ctest --preset dev -R frontend
```
