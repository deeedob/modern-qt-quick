set(APP_USE_VCPKG_QT ON CACHE BOOL "" FORCE)
set(APP_PREVIOUS_USE_VCPKG_QT OFF CACHE INTERNAL "" FORCE)
set(VCPKG_INSTALLED_DIR "/vcpkg/installed")
set(VCPKG_TARGET_TRIPLET "x64-test")
set(Qt6_DIR "/previous/Qt6" CACHE PATH "" FORCE)
set(Qt6Core_DIR "/previous/Qt6Core" CACHE PATH "" FORCE)

include("${SELECT_QT_MODULE}")
app_configure_qt_source()

if(NOT Qt6_ROOT STREQUAL "/vcpkg/installed/x64-test")
    message(FATAL_ERROR "Qt6_ROOT did not select the vcpkg Qt prefix")
endif()

if(DEFINED CACHE{Qt6_DIR} OR DEFINED CACHE{Qt6Core_DIR})
    message(FATAL_ERROR "Qt discovery cache entries were not cleared")
endif()
