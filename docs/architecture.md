# Architecture

This document explains how the source tree is organized and why. For step-by-step changes,
see the [how-to guides](how-to.md).

## Libraries outside, features inside

The layout follows [Pitchfork](https://github.com/vector-of-bool/pitchfork), adapted to an
application:

| Folder | Holds | Depends on |
|---|---|---|
| `libs/<name>/` | separable libraries: plain C++, each with `include/`, `src/` and `tests/` | the standard library and non-Qt libraries |
| `src/<feature>/` | the application, one QML module per feature, C++ and QML side by side | `libs/`, Qt, other features |
| `tests/<feature>/` | tests for `src/`, mirroring its folders | what they test |

Code belongs in `libs/` when someone could use it without this application. The shape
recognizer passes that test: games, gesture input and command-line tools recognize strokes
too. The canvas's tessellation does not, so it stays private to `src/canvas`. A library that
only holds the app's own types, or a helper of one feature, would split a feature across
folders without making anything reusable.

Inside `src/`, a feature lives in one folder: its store or view model, its item model, its
storage, its custom items and its pages. Adding a feature adds one folder in `src/` and one in
`tests/`; deleting it deletes those two.

## How the build enforces the boundaries

Every boundary is a `target_link_libraries` call, so the compiler rejects a violation:

- `libs/shapes` links nothing from Qt and switches moc off. A Qt include there fails to compile.
- Dependencies between features point one way: `sketchbook` links `canvas`, `canvas` links
  `shapes`, and neither links back.
- `src/style` links no feature, so it could move to its own repository unchanged.
- `src/app/main.cpp` is the composition root. It is the only file that knows every concrete
  type, creates the objects, and hands them to QML.

Within a feature module, keeping view models free of visual types is a convention: the module
links Qt Quick for its custom item. The tests enforce it in practice, because the stores and
view models are tested with plain Qt Test, without a window.

## The separable library

`libs/shapes` implements the [$1 Unistroke Recognizer](https://depts.washington.edu/acelab/proj/dollar/index.html)
(Wobbrock, Wilson and Li, UIST 2007), a short paper that students can read alongside the code.
One change from the paper: strokes are scaled to a square before they are rotated, so an
ellipse matches the circle template and a tall rectangle the square one. Its API is three
types in one header:

```cpp
shapes::Recognizer recognizer;
recognizer.addTemplate("circle", circleOutline);  // any std::span<const shapes::Point>
const std::optional<shapes::Match> match = recognizer.recognize(stroke);
// match->name == "circle", match->score between 0 and 1
```

Data crosses the boundary as `std::span<const shapes::Point>` going in and plain values
coming out. The library allocates its own working copies, keeps no global state, and never
calls back, so it is safe to call from any thread. Its tests use Catch2 and run without Qt;
a hidden benchmark measures recognition speed:

```sh
build/dev/libs/shapes/tests/tst_shapes "[benchmark]"
```

This is how applications with real algorithms are organized, from photo editors to audio
plugins: the digital signal processing, analysis or geometry lives in plain C++ with its own
tests, and the Qt side only feeds it data and shows its results.

## Threads

The GUI thread owns every QObject. Two other threads touch application data:

- **The render thread** calls `StrokeItem::updatePaintNode()` while the GUI thread waits, so
  the item reads the store's strokes there without locking. It builds geometry from them and
  must not touch them after returning.
- **The storage thread** writes sketch files. `SketchStorage::save()` hands a snapshot of the
  sketch to `QtConcurrent::run()` on a pool with one thread, so writes keep their order. The
  snapshot shares its strokes with the canvas instead of copying them, which is safe because
  a stroke never changes once drawn (`std::shared_ptr<const Stroke>`). Reads wait for pending
  writes first, so they never see an old file.

Immutable data shared by pointer is the general recipe for handing work to another thread:
nothing to lock, and nothing to copy.

## Logging and crash handling

Every module logs through its own `QLoggingCategory` (`app.canvas`, `app.storage`, ...),
declared with `Q_STATIC_LOGGING_CATEGORY` in the one file that uses it.
`src/app/Logging.cpp` installs a message handler that writes each line to the console and to
a session log file under the app's data directory, keeping the newest ten sessions. The first
lines of every log record the version, git commit, compiler, Qt build and operating system,
so a log file from a user is enough to reproduce their setup.

Enable debug output at runtime without recompiling:

```sh
QT_LOGGING_RULES="app.*.debug=true" ./QtAppTemplate
```

The same function replaces `std::terminate`. An uncaught exception is logged with its
message and a symbolized stack trace from cpptrace before the process aborts. Fatal Qt
messages (`qFatal`, a failed `Q_ASSERT`) get the same stack trace.

Segmentation faults are not handled. A signal handler may only call async-signal-safe
functions, which rules out symbolizing a stack trace in-process; cpptrace supports a safe
variant only on Linux with glibc 2.35 or newer. The operating system's crash reporter
(macOS crash reports, Windows Error Reporting, systemd-coredump) covers that case without
undefined behavior.

## Growing the application

The structure grows by repetition, not by new concepts:

- Reusable algorithms go into a new `libs/<name>`, with its own tests.
- A new feature adds `src/<feature>` and `tests/<feature>`, plus one line in `main()` and in
  `Main.qml`.
- Reusable visual components go into `src/style`.
