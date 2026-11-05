app_add_qt_test(tst_CanvasStore SOURCES tst_CanvasStore.cpp LIBRARIES app_canvas)
app_add_quick_test(tst_canvas_quick QML_FILES tst_StrokeItem.qml LIBRARIES app_canvasplugin)
# QWindowSystemInterface (a private Qt API, as in Qt's own tests) injects trackpad gestures.
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
app_add_qt_test(tst_CanvasPage SOURCES tst_CanvasPage.cpp
    LIBRARIES Qt::Quick Qt::GuiPrivate app_canvas app_canvasplugin app_styleplugin)
