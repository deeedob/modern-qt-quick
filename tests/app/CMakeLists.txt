app_add_qt_test(tst_Logging SOURCES tst_Logging.cpp LIBRARIES app_support)

get_target_property(_build_info_header app_support APP_BUILD_INFO_HEADER)
app_add_qt_test(tst_AppController
    SOURCES
        tst_AppController.cpp "${_build_info_header}"
        ../../src/app/AppController.cpp ../../src/app/AppController.hpp
    LIBRARIES
        app_support
        Qt::Gui
)

app_add_qt_test(tst_Threading SOURCES tst_Threading.cpp LIBRARIES Qt::Concurrent)
