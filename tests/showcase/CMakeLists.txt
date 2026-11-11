qt_add_executable(showcase showcase.cpp)
target_link_libraries(showcase PRIVATE app_ui app_uiplugin app_controlsplugin app_canvas app_canvasplugin
    app_sketchbook app_sketchbookplugin Qt::Quick Qt::Test Qt::Svg)
app_target_defaults(showcase)
