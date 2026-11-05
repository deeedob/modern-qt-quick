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
| Licensing | `REUSE.toml` | `check_reuse` |

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

`APP_SANITIZERS` takes a list of sanitizers; the `asan` preset sets `address;undefined`. The
compiler rejects combinations that cannot work together, such as thread with address. Qt itself is not instrumented, so the test preset disables AddressSanitizer's
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
conflict with Qt idioms (moc, logging macros), common C++ idioms (the `Overloaded` visitor),
or the formatting style. Identifier naming is left to each project. Where a check is right in
general but wrong in one known place, `.clang-tidy` configures it instead of the code carrying
`NOLINT` comments: item-model roles, for example, stay unscoped enums inside their model
(`cppcoreguidelines-use-enum-class.IgnoreUnscopedEnumsInClasses`).

The checks cover public headers in a `FILE_SET` too. CMake keeps those out of a target's
`SOURCES`, so `app_target_defaults()` collects them from the target's header sets.

## Tests

Each part is tested with the lightest tool that can test it:

| Part | Framework | What the demo tests |
|---|---|---|
| `libs/shapes` | Catch2, in `libs/shapes/tests/` | recognition across positions, sizes, aspect ratios and directions; degenerate input; a hidden benchmark |
| `src/canvas` | Qt Test | the store: actions, undo and redo, snapping, history limits |
| `src/canvas` | Qt Quick Test | `StrokeItem` turns pointer input into a `strokeFinished` signal, also inside a `Flickable` |
| `src/canvas` | Qt Test | trackpad pinches on `CanvasPage` zoom around the fingers and stay on the paper |
| `src/sketchbook` | Qt Test | storage round trips, ordered saves, broken files; the view model with `QAbstractItemModelTester` |
| `src/style` | Qt Quick Test | the tooltip and click behavior of `ToolButton` |
| `src/app` | Qt Test | log files and their rotation |

Qt Test and Qt Quick Test cover the Qt parts, and Catch2 covers the library, because a
separable library should not need Qt even for its tests.

The test presets set `QT_QPA_PLATFORM=offscreen`, so no windows appear. That platform turns
hover effects off, which is why `tst_ToolButton.qml` enables hover on the button it tests.
