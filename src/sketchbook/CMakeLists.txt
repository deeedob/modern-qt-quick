# The list of sketches, as MVVM: a view model with an item model, and the files behind them.

qt_add_qml_module(app_sketchbook
    URI App.Sketchbook
    STATIC
    QML_FILES
        SketchbookPage.qml
    SOURCES
        SketchbookViewModel.cpp SketchbookViewModel.hpp
        SketchListModel.cpp SketchListModel.hpp
        SketchStorage.cpp SketchStorage.hpp
)
target_include_directories(app_sketchbook PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_sketchbook
    PUBLIC Qt::Qml Qt::QmlModels app_canvas
    PRIVATE glaze::glaze app_controls
)
app_target_defaults(app_sketchbook)
