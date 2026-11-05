# QML and C++

This document explains how C++ objects reach QML, the two presentation patterns the demo
uses side by side (MVVM and Flux), the custom scene graph item, and the style.

## Who owns what

`main()` creates every C++ object the UI uses, on the stack, before the QML engine:

```cpp
app::sketchbook::SketchStorage storage(dataDir + "/sketches"_L1);
app::canvas::CanvasStore canvas;
app::sketchbook::SketchbookViewModel sketchbook(storage, canvas);
app::AppController controller(logFile);

QQmlApplicationEngine engine;
```

C++ objects are destroyed in reverse order, so the engine goes first and QML never holds a
dangling pointer. QML only receives non-owning references, and the object lifetimes are
visible in one place. `SketchStorage` goes last; its destructor waits for pending saves.

## Three ways in, and when to use each

**Required properties, for feature objects.** `Main.qml` declares what it needs:

```qml
ApplicationWindow {
    required property SketchbookViewModel sketchbook
    required property CanvasStore canvas
}
```

and `main()` provides it with `engine.setInitialProperties()`. The window passes each object
down explicitly (`CanvasPage { store: window.canvas }`). Every component states its
dependencies in its own interface, so it is reusable and testable in isolation. Since Qt 6.12,
a missing required property is an error instead of a silently incomplete object.

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

## MVVM: the sketchbook

`SketchbookViewModel` exposes state as properties (`model`, `openName`) and commands as
`Q_INVOKABLE` methods (`create`, `open`, `rename`, `remove`). `SketchbookPage.qml` binds to
the properties and calls the commands; it decides nothing and knows no files. The view model
reads and writes through `SketchStorage` and publishes the rows through `SketchListModel`, a
`QAbstractListModel` that inserts, changes and removes single rows instead of resetting, so
the list keeps its scroll position.

Model data reaches the delegate as required properties (`required property string name`), and
user intent leaves through the view model's commands. Qt's model/view classes are the MVC
pattern in Qt's form: the item model is the model, the `ListView` and its delegate are the
view, and the delegate's input handling is the controller. MVVM adds the view model on top,
which turns domain operations into commands a view can call.

MVVM suits screens whose state maps directly onto controls, such as lists and forms.

## Flux: the canvas

The canvas follows a unidirectional data flow:

```
CanvasPage ──addStroke(), undo(), ...──▶ CanvasStore ──reduce(sketch, action)──▶ new sketch ──changed──▶ CanvasPage, StrokeItem
```

- The state is a plain value: `Sketch`, a list of strokes. A stroke is
  `std::shared_ptr<const Stroke>` and never changes once drawn.
- Every change is an action: `AddStroke`, `ReplaceLastStroke`, `Clear`.
- `reduce()` in `Sketch.hpp` computes the next sketch. It is a pure function of plain C++:
  the same sketch and action always give the same result. `std::visit` with one lambda per
  action makes a forgotten action a compile error.
- `CanvasStore` keeps every sketch that `reduce()` returned. Undo and redo move through that
  history; no action needs an inverse. Since sketches share their strokes, a history step
  costs one pointer per stroke.
- A recognized shape is a second action: `AddStroke` puts the stroke as drawn, then
  `ReplaceLastStroke` puts the clean shape. One undo brings back the drawn stroke.
- Views cannot change state except through the store's actions. Its properties are read-only.

Flux costs more ceremony than MVVM, and pays off when the history of changes matters: undo,
redo, autosave of every edit, or replaying what led to a bug. `QT_LOGGING_RULES="app.canvas.debug=true"`
logs every snap with its score.

## A custom scene graph item

`StrokeItem` subclasses `QQuickItem`, sets `ItemHasContents`, and builds its geometry in
`updatePaintNode()`. That function runs on the render thread while the GUI thread waits, so it
may read the store and the item's members, but must create only scene graph nodes. Each stroke
becomes one `QSGGeometryNode` with a triangle strip and a flat color material.

Because strokes never change, the item reuses a node for as long as the same stroke is in the
same place. After a new stroke, only one node is built; after an undo, nodes are only removed.
The item keeps the strokes it rendered as `shared_ptr`s, so a freed stroke's address cannot be
reused by a new one and fool that comparison.

The item reads the store but never writes to it. Pointer input becomes a `strokeFinished`
signal, and `CanvasPage.qml` turns that into an action. The item stays a reusable drawing
surface.

The alternatives, and why the template does not use them:

- `QQuickPaintedItem` draws with `QPainter` into a texture. It is simpler but rasterizes the
  whole item again on every change.
- `QQuickRhiItem` renders with the low-level graphics API and custom shaders. It is meant for
  3D and heavy rendering.
- Qt Quick Shapes draws vector paths from QML. It suits static graphics.

## The style

The views never import `QtQuick.Controls`. They import `AppStyle` (`src/style`), a Qt Quick
Controls style built the way the [Qt documentation recommends](https://doc.qt.io/qt-6/qtquickcontrols-customize.html):

- Each customized control is a QML file whose root is the control's template, for example
  `T.Button` from `QtQuick.Templates`. The style defines only the controls the app uses:
  `ApplicationWindow`, `Button`, `ToolButton`, `TextField`, `ItemDelegate` and `ToolTip`.
- Every other control comes from the Basic style, through `IMPORTS QtQuick.Controls.Basic` in
  the module. Basic is the style meant to be customized; the native macOS and Windows styles
  are not.
- `Theme.qml` holds the design tokens: colors, spacing, radius, font and icons. It follows the
  system's light or dark mode through `Application.styleHints.colorScheme`.
- `ApplicationWindow.qml` sets the window's `palette` from the tokens. The palette reaches
  every control, so the controls taken from Basic (`Dialog`, `Switch`, `Label`) match too.

Importing the style module directly selects the style at compile time, so the QML compiler
compiles bindings on controls to C++ and qmllint knows every type. Runtime style selection
(`QQuickStyle::setStyle()`) would lose both.

An icon-only `ToolButton` shows its `text` as a tooltip. The views set `text` on every icon
button, which doubles as its accessible name.

Qt 6.11 introduced Qt Labs StyleKit, which describes a whole style in one declarative file with
built-in themes and states. It is a technology preview, and its API changed between 6.11 and
6.12, so this template stays with the stable approach for now.

## QML module rules this template follows

- One QML module per feature folder, created with `qt_add_qml_module()` and a `STATIC`
  backing library. Static modules register only when their plugin target is linked, which is
  why `app` and the tests link `*plugin` targets.
- URIs follow `App.<Feature>`; the style is `AppStyle`. `QT_QML_OUTPUT_DIRECTORY` collects
  every module under `build/<preset>/qml/`, so source folders can stay lowercase.
- Every QML file that declares delegates or inline components starts with
  `pragma ComponentBehavior: Bound`, uses typed properties and function signatures, and
  qualifies every lookup (`root.store`, never a bare `store`). `all_qmllint` reports
  violations.
- The project compiles with `QT_NO_KEYWORDS`, so C++ writes `Q_SIGNALS`, `Q_SLOTS` and
  `Q_EMIT`. Qt's `signals`, `slots` and `emit` are macros that break third-party headers using
  those names; glaze, for example, has a variable called `emit`.
