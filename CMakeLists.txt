cmake_minimum_required(VERSION 3.31)

include(cmake/SelectQt.cmake)

project(Sketchbook
    VERSION 0.1.0
    DESCRIPTION "A sketchbook that recognizes shapes: a Qt Kickstart template"
    LANGUAGES CXX
)

set(APP_DISPLAY_NAME "Sketchbook" CACHE STRING "Human-readable application name")
set(APP_BUNDLE_ID "org.example.Sketchbook" CACHE STRING "Reverse-DNS application identifier")
option(BUILD_TESTING "Build the test suites" ON)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# The project uses no C++ modules. Scanning for them makes GCC compile with its experimental
# -fmodules-ts, which has crashed GCC 14 on glaze's headers.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(AppTargetDefaults)
include(BuildInfo)
include(Packaging)

app_configure_qt_source()
find_package(Qt6 6.12 COMPONENTS
    Core Gui Qml QmlModels Quick QuickControls2 Concurrent Svg LinguistTools Test QuickTest
)
if(NOT Qt6_FOUND)
    message(FATAL_ERROR
        "Qt 6.12 or newer was not found.\n"
        "Install it with the Qt Online Installer or aqtinstall, then point QT_ROOT_DIR at it:\n"
        "  export QT_ROOT_DIR=~/Qt/6.12.0/macos\n"
        "Alternatively, enable APP_USE_VCPKG_QT to build Qt with vcpkg.")
endif()
app_validate_qt_source()

qt_standard_project_setup(REQUIRES 6.12
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES de
)
# One QML import root in the build tree: source folders stay lowercase, URIs stay PascalCase.
set(QT_QML_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/qml")

find_package(cpptrace CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED) # demo

add_subdirectory(libs/shapes) # demo
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include(Checks)
