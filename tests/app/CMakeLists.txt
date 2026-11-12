app_add_qt_test(tst_LogSession SOURCES tst_LogSession.cpp LIBRARIES app_support)

app_add_qt_test(tst_CloseRecovery SOURCES tst_CloseRecovery.cpp
    LIBRARIES app_support app_sketchbook app_canvas app_uiplugin app_controlsplugin
        app_canvasplugin app_sketchbookplugin app_notify app_notifyplugin
        Qt::Gui Qt::Qml Qt::Quick)
target_compile_definitions(tst_CloseRecovery PRIVATE APP_SOURCE_DIR="${PROJECT_SOURCE_DIR}")


app_add_qt_test(tst_AppController
    SOURCES
        tst_AppController.cpp
    LIBRARIES
        app_ui
        app_uiplugin
        Qt::Gui
)
