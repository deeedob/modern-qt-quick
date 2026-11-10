# The single source of compile policy for first-party targets:
#
#   app_target_defaults(<target>)
#       Strict warnings on the target's own sources, hardening, sanitizers, and
#       registration for the `check` targets. Call it once for every first-party
#       target; third-party and Qt-generated code is never affected.
#
# Warnings-as-errors is not handled here: presets set CMAKE_COMPILE_WARNING_AS_ERROR,
# and `cmake --compile-no-warning-as-error` turns it off locally.

include_guard(GLOBAL)
include(CheckCXXCompilerFlag)

option(APP_ENABLE_HARDENING "Enable standard library assertions and stack/fortify hardening" ON)
set(APP_SANITIZERS "" CACHE STRING "Sanitizers for first-party targets, e.g. \"address;undefined\"")

if(MSVC)
    set(_app_warnings
        /W4 /permissive- /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311
        /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928
    )
else()
    set(_app_warnings
        -Wall -Wextra -Wpedantic
        -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual
        -Wnull-dereference -Wimplicit-fallthrough
        -Wold-style-cast -Wcast-align -Wconversion -Wsign-conversion -Wdouble-promotion
        -Wunused -Wformat=2
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND _app_warnings
            -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast
        )
    endif()
endif()

set(_app_hardening_options "")
set(_app_hardening_definitions "")
set(_app_hardening_link_options "")
if(APP_ENABLE_HARDENING)
    if(MSVC)
        list(APPEND _app_hardening_options /sdl /guard:cf)
        # Control Flow Guard only takes effect when the linker emits the guard tables too.
        list(APPEND _app_hardening_link_options /GUARD:CF)
    else()
        # Each define is ignored by the other standard library, so both are always safe to set.
        list(APPEND _app_hardening_definitions
            _GLIBCXX_ASSERTIONS
            _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST
        )
        # -Werror: AppleClang accepts some of these flags but only warns that they are unused.
        set(CMAKE_REQUIRED_FLAGS -Werror)
        foreach(_flag IN ITEMS -fstack-protector-strong -fcf-protection -fstack-clash-protection)
            string(MAKE_C_IDENTIFIER "APP_HAS${_flag}" _var)
            check_cxx_compiler_flag(${_flag} ${_var})
            if(${_var})
                list(APPEND _app_hardening_options ${_flag})
            endif()
        endforeach()
        unset(CMAKE_REQUIRED_FLAGS)
        # Distro compilers often predefine _FORTIFY_SOURCE=2; fortification needs an optimizer.
        list(APPEND _app_hardening_options
            $<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE>
            $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=3>
        )
    endif()
endif()

set(_app_sanitizer_options "")
if(APP_SANITIZERS)
    if(MSVC)
        if(NOT APP_SANITIZERS STREQUAL "address")
            message(FATAL_ERROR "APP_SANITIZERS: MSVC only supports \"address\"")
        endif()
        set(_app_sanitizer_options /fsanitize=address)
    else()
        list(JOIN APP_SANITIZERS "," _list)
        set(_app_sanitizer_options -fsanitize=${_list} -fno-omit-frame-pointer -fno-sanitize-recover=all)
    endif()
endif()

function(app_target_defaults target)
    target_compile_options(${target} PRIVATE ${_app_hardening_options} ${_app_sanitizer_options})
    target_compile_definitions(${target} PRIVATE ${_app_hardening_definitions}
        # Qt's `emit`, `signals` and `slots` are macros that break third-party code using those
        # names (glaze has a variable called emit). Write Q_EMIT, Q_SIGNALS and Q_SLOTS instead.
        QT_NO_KEYWORDS
        # Disables the Qt APIs that Qt itself deems dangerous or deprecated, up to the oldest Qt
        # this project supports: keep it equal to the version in find_package(Qt6).
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,12,0)"
        # Not part of strict mode: no implicit conversion from char * to QString, whose encoding
        # the compiler cannot know. Write u"..."_s or "..."_L1 instead.
        QT_NO_CAST_FROM_ASCII
    )
    target_link_options(${target} PRIVATE ${_app_hardening_link_options})
    if(_app_sanitizer_options AND NOT MSVC)
        target_link_options(${target} PRIVATE ${_app_sanitizer_options})
    endif()
    set_property(GLOBAL APPEND PROPERTY APP_FIRST_PARTY_TARGETS ${target})
    # Deferred to the end of the top-level directory, so sources added later are included.
    # EVAL bakes in the target name now: deferred arguments are otherwise expanded at call time.
    cmake_language(EVAL CODE
        "cmake_language(DEFER DIRECTORY [[${PROJECT_SOURCE_DIR}]] CALL _app_apply_warnings [[${target}]])")
endfunction()

# Warnings go on hand-written sources only: Qt compiles generated code (moc, type
# registrations, AOT-compiled QML) into the same targets, and we do not own that code.
function(_app_apply_warnings target)
    get_target_property(_sources ${target} SOURCES)
    get_target_property(_source_dir ${target} SOURCE_DIR)
    # Headers in a FILE_SET are not part of SOURCES, but the checks must see them too.
    get_target_property(_header_sets ${target} HEADER_SETS)
    if(_header_sets)
        foreach(_set IN LISTS _header_sets)
            get_target_property(_headers ${target} HEADER_SET_${_set})
            list(APPEND _sources ${_headers})
        endforeach()
    endif()
    set(_compiled "")
    foreach(_src IN LISTS _sources)
        if(_src MATCHES "^\\$<")
            continue()
        endif()
        cmake_path(ABSOLUTE_PATH _src BASE_DIRECTORY "${_source_dir}" NORMALIZE)
        cmake_path(IS_PREFIX PROJECT_BINARY_DIR "${_src}" NORMALIZE _generated)
        if(_generated)
            continue()
        endif()
        set_property(GLOBAL APPEND PROPERTY APP_FIRST_PARTY_SOURCES "${_src}")
        if(_src MATCHES "\\.cpp$")
            list(APPEND _compiled "${_src}")
        endif()
    endforeach()
    if(_compiled)
        set_property(SOURCE ${_compiled} TARGET_DIRECTORY ${target}
            APPEND PROPERTY COMPILE_OPTIONS ${_app_warnings})
    endif()
endfunction()
