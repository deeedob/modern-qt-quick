<!--
SPDX-FileCopyrightText: 2026 Dennis Oberst
SPDX-License-Identifier: MIT-0
-->

# QML and C++

This document explains how C++ objects reach QML in this template, and the two
presentation patterns the demo compares: MVVM and Flux.

## Who owns what

`main()` creates every C++ object the UI uses, on the stack, before the QML engine:

```cpp
app::platform::JsonTaskRepository repository(dataDir + u"/tasks.json"_s);
app::backend::TasksViewModel tasks(repository);
app::backend::TimerStore timer;
app::AppController controller(logFile);

QQmlApplicationEngine engine;
```

C++ objects are destroyed in reverse order, so the engine goes first and QML never holds a
dangling pointer. QML only receives non-owning references, and the object lifetimes are
visible in one place.

## Three ways in, and when to use each

**Required properties, for feature objects.** `Main.qml` declares what it needs:

```qml
ApplicationWindow {
    required property TasksViewModel tasks
    required property TimerStore timer
}
```

and `main()` provides it with `engine.setInitialProperties()`. The window passes each object
down explicitly (`TasksPage { viewModel: window.tasks }`). Every component states its
dependencies in its own interface, so it is reusable and testable in isolation: a test can
pass a different object. Since Qt 6.12, a missing required property is an error instead of a
silently incomplete object.

**An external singleton, for truly app-wide state.** The language switch and the build
information are needed everywhere, and threading them through every component would be noise.
`AppController` is declared `QML_SINGLETON` and `QML_UNCREATABLE`, and `main()` registers the
instance it constructed:

```cpp
engine.setExternalSingletonInstance("App", "AppController", &controller);
```

QML code then writes `AppController.language`. Unlike a classic singleton, this one has no
global state in C++: `main()` creates it with its real dependencies, and a test can create
another. The function was added in Qt 6.12; before that, the same effect needed a static
`create()` factory.

**Never context properties.** `rootContext()->setContextProperty()` injects untyped names that
qmllint and the QML compiler cannot see. The Qt documentation advises against it, and qmllint
reports it.

Types that QML only receives, never creates, are marked `QML_UNCREATABLE` with a message that
says where the instance comes from.

## MVVM: the tasks feature

`TasksViewModel` exposes state as properties (`model`, `searchText`, `openCount`) and actions
as `Q_INVOKABLE` methods (`add`, `toggle`, `remove`). `TasksPage.qml` binds to the properties
and calls the methods; it makes no decisions of its own. The view model filters and ranks the
tasks with `core::rankTasks()` and publishes the visible rows through `TaskListModel`, a
`QAbstractListModel`.

Model data reaches the delegate as required properties (`required property string title`),
and the delegate reports user intent as signals (`toggleRequested`). The delegate never
writes to the model, so it works with any model that has the same roles.

MVVM suits screens with a lot of state that maps directly onto controls.

## Flux: the focus timer

The timer follows a unidirectional data flow:

```
TimerPage ──dispatch(Action)──▶ TimerStore ──reduce(state, action)──▶ new state ──stateChanged──▶ TimerPage
```

- The state is one plain struct, `core::TimerState`.
- Every change is an action: `Start`, `Pause`, `Reset`, `Tick`, `SetDuration`.
- `core::reduce()` computes the next state. It is a pure `constexpr` function in `libs/core`,
  without clocks, timers or Qt, so its tests run as `static_assert`s.
- `TimerStore` is the only place with side effects: it starts and stops the tick timer and
  emits `stateChanged` and `finished`. Its properties are read-only, so a view cannot
  change state except by dispatching an action.

Flux costs more ceremony than MVVM, and pays off when state changes come from several
sources (user input, timers, network) and must stay predictable. Every change is logged as
an action (`QT_LOGGING_RULES="app.timer.debug=true"`), which makes the history of a bug
easy to read.

The two patterns combine well: a view model can dispatch to a store behind the scenes.

## Custom components

**Pure QML first.** `IconButton.qml` in `frontend/ui` wraps a standard control behind a small
typed API. Most custom components need nothing more.

**A C++ scene graph item for custom drawing.** `ProgressRing` subclasses `QQuickItem`, sets
`ItemHasContents`, and builds its geometry in `updatePaintNode()`. That function runs on the
render thread while the GUI thread waits, so it may read the item's members but must create
only scene graph nodes. Two arcs, the track and the progress, are triangle strips with a flat
color material.

The alternatives, and why the template does not use them:

- `QQuickPaintedItem` draws with `QPainter` into a texture. It is simpler but slower, since
  every change rasterizes the whole item again.
- `QQuickRhiItem` renders with the low-level graphics API. It is meant for 3D and custom
  shaders.
- Qt Quick Shapes draws vector paths from QML. It suits static graphics; the ring shows the
  C++ path on purpose.

Custom geometry needs a GPU backend. Qt Quick's software renderer, which the `offscreen`
platform uses, does not draw it, so the pixel test in `tests/frontend/tst_ProgressRing.qml`
skips itself there.

## QML module rules this template follows

- One QML module per directory, created with `qt_add_qml_module()` and a `STATIC` backing
  library. Static modules register only when their plugin target is linked, which is why
  `app` and the tests link `*plugin` targets.
- URIs follow `App.<Layer>.<Module>`. `QT_QML_OUTPUT_DIRECTORY` collects every module under
  `build/<preset>/qml/`, so source folders can stay lowercase.
- Every QML file starts with `pragma ComponentBehavior: Bound`, uses typed properties and
  function signatures, and qualifies every lookup (`root.viewModel`, never a bare
  `viewModel`). `all_qmllint` reports violations.
- Controls come from `QtQuick.Controls.Material`, imported explicitly so the QML compiler can
  resolve them at build time.
