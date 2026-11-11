app_add_qt_test(tst_CanvasStore SOURCES tst_CanvasStore.cpp LIBRARIES app_canvas)
app_add_quick_test(tst_canvas_quick QML_FILES tst_StrokeItem.qml tst_SketchTitle.qml LIBRARIES app_canvasplugin app_controlsplugin)
app_add_qt_test(tst_CanvasPage SOURCES tst_CanvasPage.cpp
    LIBRARIES Qt::Quick app_canvas app_canvasplugin app_controlsplugin)
