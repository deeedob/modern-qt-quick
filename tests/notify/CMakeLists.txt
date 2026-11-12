app_add_qt_test(tst_Center SOURCES tst_Center.cpp LIBRARIES app_notify)
app_add_qt_test(tst_Host SOURCES tst_Host.cpp
    LIBRARIES app_notify app_notifyplugin app_controlsplugin Qt::Quick)
