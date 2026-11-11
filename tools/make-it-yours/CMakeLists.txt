find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME test_make_it_yours
        COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/test_make_it_yours.py")
else()
    message(STATUS "Template setup tests unavailable: Python 3 was not found")
endif()
