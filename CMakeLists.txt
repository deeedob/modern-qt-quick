# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

cmake_minimum_required(VERSION 3.31)

include(cmake/PreventInSourceBuilds.cmake)

# Must precede project().
include(cmake/MacOSDeploymentTarget.cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET "${APP_MACOS_DEPLOYMENT_TARGET}" CACHE STRING "Oldest supported macOS")

project(QtAppTemplate
    VERSION 0.1.0
    DESCRIPTION "Tasks and a focus timer: a modern Qt Quick application template"
    LANGUAGES CXX
)

set(APP_DISPLAY_NAME "Focus Tasks" CACHE STRING "Human-readable application name")
set(APP_BUNDLE_ID "org.example.QtAppTemplate" CACHE STRING "Reverse-DNS application identifier")
option(BUILD_TESTING "Build the test suites" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(AppTargetDefaults)
include(BuildInfo)

find_package(Qt6 6.12 COMPONENTS
    Core Gui Qml QmlModels Quick QuickControls2 Svg LinguistTools Test QuickTest
)
if(NOT Qt6_FOUND)
    message(FATAL_ERROR
        "Qt 6.12 or newer was not found.\n"
        "Install it with the Qt Online Installer or aqtinstall, then point QT_ROOT_DIR at it:\n"
        "  export QT_ROOT_DIR=~/Qt/6.12.0/macos\n"
        "Alternatively let vcpkg build Qt (takes hours): cmake --workflow --preset vcpkg-qt")
endif()

qt_standard_project_setup(REQUIRES 6.12
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES de
)
# One QML import root in the build tree: source folders stay lowercase, URIs stay PascalCase.
set(QT_QML_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/qml")

find_package(rapidfuzz CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)

if(BUILD_TESTING)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
endif()

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include(Packaging)
include(Checks)
