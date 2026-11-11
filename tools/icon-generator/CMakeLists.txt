if(CMAKE_CROSSCOMPILING)
    message(STATUS "Skipping the icon generator during cross-compilation")
    return()
endif()

qt_add_executable(app_icon_generator IconGenerator.cpp)
target_link_libraries(app_icon_generator PRIVATE Qt::Core Qt::Gui Qt::Svg)
app_target_defaults(app_icon_generator)
