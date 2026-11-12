find_package(Qt6 6.12 QUIET COMPONENTS Test)
if(NOT TARGET Qt6::Test)
    message(STATUS "Showcase tool unavailable: Qt Test was not found")
    return()
endif()

qt_add_executable(showcase EXCLUDE_FROM_ALL showcase.cpp)
target_link_libraries(showcase PRIVATE app_ui app_uiplugin app_controlsplugin app_canvas app_canvasplugin
    app_sketchbook app_sketchbookplugin app_notify app_notifyplugin
    Qt::Quick Qt::Test Qt::Svg)
app_target_defaults(showcase)
