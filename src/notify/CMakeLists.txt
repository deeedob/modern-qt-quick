qt_add_qml_module(app_notify
    URI App.Notify
    STATIC
    QML_FILES
        ActionButton.qml
        Card.qml
        DecisionDialog.qml
        NotificationHost.qml
    SOURCES
        Message.cpp Message.hpp
        NotificationCenter.cpp NotificationCenter.hpp
)
target_include_directories(app_notify PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(app_notify PUBLIC Qt::Core Qt::Qml PRIVATE Qt::Quick app_controls)
app_target_defaults(app_notify)
