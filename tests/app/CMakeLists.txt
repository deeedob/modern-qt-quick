app_add_qt_test(tst_Logging SOURCES tst_Logging.cpp LIBRARIES app_support)

app_add_qt_test(tst_MainErrorFlow SOURCES tst_MainErrorFlow.cpp
    LIBRARIES app_support app_sketchbook app_canvas app_uiplugin app_controlsplugin
        app_canvasplugin app_sketchbookplugin Qt::Gui Qt::Qml Qt::Quick)
target_compile_definitions(tst_MainErrorFlow PRIVATE APP_SOURCE_DIR="${PROJECT_SOURCE_DIR}")


app_add_qt_test(tst_AppController
    SOURCES
        tst_AppController.cpp
    LIBRARIES
        app_ui
        Qt::Gui
)

app_add_qt_test(tst_Threading SOURCES tst_Threading.cpp LIBRARIES Qt::Concurrent)
