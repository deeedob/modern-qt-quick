# Drawing, as a Flux store, with the custom scene graph item that renders the strokes.

qt_add_qml_module(app_canvas
    URI App.Canvas
    STATIC
    QML_FILES
        CanvasPage.qml
        CanvasSettings.qml
        SketchTitle.qml
    SOURCES
        CanvasStore.cpp CanvasStore.hpp
        Sketch.hpp
        StrokeItem.cpp StrokeItem.hpp
)
target_include_directories(app_canvas PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_canvas
    PUBLIC Qt::Quick shapes::shapes
    PRIVATE app_controls
)
app_target_defaults(app_canvas)
