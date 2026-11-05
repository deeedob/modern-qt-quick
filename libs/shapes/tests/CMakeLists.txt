find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)

add_executable(tst_shapes tst_Recognizer.cpp)
target_link_libraries(tst_shapes PRIVATE shapes::shapes Catch2::Catch2WithMain)
app_target_defaults(tst_shapes)
catch_discover_tests(tst_shapes TEST_PREFIX "shapes." DISCOVERY_MODE PRE_TEST)
