# Tests for src/, mirroring its folders. Libraries in libs/ carry their own tests.
#   C++   Qt Test, one executable per class
#   QML   Qt Quick Test, one tst_*.qml file per component, run by app_add_quick_test()

set_directory_properties(PROPERTIES QT_EXCLUDE_FROM_TRANSLATION ON)

add_test(NAME tst_SelectQt
    COMMAND "${CMAKE_COMMAND}"
        "-DSELECT_QT_MODULE=${PROJECT_SOURCE_DIR}/cmake/SelectQt.cmake"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tst_SelectQt.cmake"
)

function(app_add_qt_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;LIBRARIES")
    qt_add_executable(${name} ${arg_SOURCES})
    target_link_libraries(${name} PRIVATE Qt::Test ${arg_LIBRARIES})
    app_target_defaults(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Static QML modules register only when their plugin is linked: pass the *plugin targets.
function(app_add_quick_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "QML_FILES;LIBRARIES")
    qt_add_executable(${name} "${PROJECT_SOURCE_DIR}/tests/quicktest.cpp" ${arg_QML_FILES})
    target_compile_definitions(${name} PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(${name} PRIVATE Qt::QuickTest ${arg_LIBRARIES})
    app_target_defaults(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_subdirectory(app)
add_subdirectory(showcase)
add_subdirectory(canvas) # demo
add_subdirectory(sketchbook) # demo
add_subdirectory(controls)
