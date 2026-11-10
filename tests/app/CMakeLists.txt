app_add_qt_test(tst_Logging SOURCES tst_Logging.cpp LIBRARIES app_support)

app_add_qt_test(tst_AppController
    SOURCES
        tst_AppController.cpp
        ../../src/app/AppController.cpp ../../src/app/AppController.hpp
    LIBRARIES
        app_support
        Qt::Gui
)
