<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# Architecture

This document explains how the source tree is organized and why. For step-by-step changes,
see the [how-to guides](how-to.md).

## Layers on the outside, features on the inside

The top level of `src/` sorts code by what it is allowed to depend on. The second level sorts
it by feature.

| Layer | Depends on | Links | Holds |
|---|---|---|---|
| `libs/` | nothing | the C++ standard library, non-Qt libraries | domain types and rules |
| `platform/` | `libs` | `Qt::Core` | storage, logging, crash handling, build info |
| `backend/<feature>/` | `libs` | `Qt::Qml`, never `Qt::Quick` | view models, stores, item models |
| `frontend/<module>/` | `backend` | `Qt::Quick` | QML views, the design system, visual C++ items |
| `app/` | everything | | `main()`, the window shell, app-wide state |

Every arrow is a `target_link_libraries` call, so the compiler rejects a violation instead of a
reviewer having to spot it. A `#include <QQuickItem>` in a view model fails because the
backend does not link Qt Quick. A Qt include in `libs/` fails because nothing there links Qt,
and `libs/CMakeLists.txt` switches moc off.

The second level keeps each feature in one place: `backend/tasks/` and `frontend/tasks/`
belong together and can be deleted together. Adding a feature means adding one folder in each
layer, without touching the others.

## Why each layer exists

**`libs/` is plain C++.** Domain logic that needs no event loop, no `QString` and no QML is
the cheapest code to test, to reuse in another program (a CLI tool, a server, a different
UI), and to reason about. The demo keeps three things here: the `Task` type, fuzzy search
(using rapidfuzz), and the focus timer's reducer, a `constexpr` function that its tests run
inside the compiler. `libs/core` is a public library: its API is the headers under
`include/app/core/`, and everything else stays private to it.

**`platform/` talks to the operating system through Qt.** Files, standard paths, logging and
crash handling depend on the machine the app runs on. Keeping them out of the backend means
the view models run unchanged in tests, with fakes in place of the real thing.

**`backend/` is the application logic the views bind to.** These are QML modules that
contain only C++: they register types such as `TasksViewModel` and `TimerStore` with QML, so
the QML tooling (qmllint, the QML compiler, qmlls) knows their properties and methods. They
cannot create or touch a single visual item, which keeps them testable with plain Qt Test.

**`frontend/` is presentation.** `frontend/ui` is the design system: theme tokens, reusable
components, fonts and icons, and no knowledge of any feature. The feature modules contain
the views, which bind to backend objects and forward user intent to them.

**`app/` is the composition root.** `main.cpp` is the only file that knows every concrete type.
It creates the repository, the view models and the store, and hands them to QML. Everything
else receives its dependencies instead of looking them up.

## Dependency inversion at the storage boundary

The tasks view model needs to load and save tasks, but must not depend on `platform/`. So
`libs/core` declares what it needs, the `TaskRepository` interface, and `platform/`
implements it with `JsonTaskRepository`. `main()` passes the real implementation to the
view model; `tests/backend` passes an in-memory fake. The view model never learns where tasks
are stored.

## Logging and crash handling

Every module logs through its own `QLoggingCategory` (`app.tasks`, `app.timer`,
`app.storage`, ...), declared with `Q_STATIC_LOGGING_CATEGORY` in the one file that uses it.
`platform/Logging.cpp` installs a message handler that writes each line to the console and
to a session log file under the app's data directory, keeping the newest ten sessions. The
first lines of every log record the version, git commit, compiler, Qt build and operating
system, so a log file from a user is enough to reproduce their setup.

Enable debug output at runtime without recompiling:

```sh
QT_LOGGING_RULES="app.*.debug=true" ./QtAppTemplate
```

`platform/CrashHandler.cpp` replaces `std::terminate`. An uncaught exception is logged with
its message and a symbolized stack trace from cpptrace before the process aborts. Fatal Qt
messages (`qFatal`, a failed `Q_ASSERT`) get the same stack trace. Run the app with
`--crash-test` to see it work.

Segmentation faults are not handled. A signal handler may only call async-signal-safe
functions, which rules out symbolizing a stack trace in-process; cpptrace supports a safe
variant only on Linux with glibc 2.35 or newer. The operating system's crash reporter
(macOS crash reports, Windows Error Reporting, systemd-coredump) covers that case without
undefined behavior.

## Growing the application

The structure grows by repetition, not by new concepts:

- More domain code goes into `libs/core`, or a new `libs/<name>` once it has its own purpose.
  Large existing C++ code bases fit here unchanged.
- More operating-system integration goes into `platform/`, split into `platform/<name>` when
  it grows.
- A new feature adds `backend/<feature>` and `frontend/<feature>`, plus one line in `main()`
  and in `Main.qml`.
- Reusable visual components go into `frontend/ui`. If several applications share them,
  the module can move to its own repository unchanged, since it imports nothing from the app.
