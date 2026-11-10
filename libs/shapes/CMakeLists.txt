# A separable library: plain C++, no Qt, its own tests. The API is include/shapes/, the rest private.

set(CMAKE_AUTOMOC OFF)

add_library(shapes STATIC)
add_library(shapes::shapes ALIAS shapes)

target_sources(shapes
    PRIVATE
        src/Recognizer.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/shapes/Recognizer.h
)
target_compile_features(shapes PUBLIC cxx_std_23)
app_target_defaults(shapes)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
