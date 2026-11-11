option(APP_USE_VCPKG_QT "Build Qt with vcpkg instead of using an installed Qt" OFF)

if((DEFINED APP_PREVIOUS_USE_VCPKG_QT AND
    NOT APP_USE_VCPKG_QT STREQUAL APP_PREVIOUS_USE_VCPKG_QT) OR
   (NOT DEFINED APP_PREVIOUS_USE_VCPKG_QT AND APP_USE_VCPKG_QT))
    get_cmake_property(cacheVariables CACHE_VARIABLES)
    foreach(cacheVariable IN LISTS cacheVariables)
        if(cacheVariable MATCHES "^(Qt6.*_DIR|_Qt6_.*|QT_FEATURE_.*)$")
            unset(${cacheVariable} CACHE)
        endif()
    endforeach()
endif()
set(APP_PREVIOUS_USE_VCPKG_QT "${APP_USE_VCPKG_QT}" CACHE INTERNAL "Previous Qt source" FORCE)

list(REMOVE_ITEM VCPKG_MANIFEST_FEATURES qt)
if(APP_USE_VCPKG_QT)
    list(APPEND VCPKG_MANIFEST_FEATURES qt)
endif()

function(app_configure_qt_source)
    if(APP_USE_VCPKG_QT)
        set(Qt6_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/Qt6" PARENT_SCOPE)
    endif()
endfunction()

function(app_validate_qt_source)
    if(NOT APP_USE_VCPKG_QT)
        cmake_path(IS_PREFIX VCPKG_INSTALLED_DIR "${Qt6_DIR}" NORMALIZE qtIsFromVcpkg)
        if(qtIsFromVcpkg)
            message(FATAL_ERROR "Found Qt in the vcpkg installation. Set QT_ROOT_DIR to an installed Qt or enable APP_USE_VCPKG_QT.")
        endif()
    endif()
endfunction()
