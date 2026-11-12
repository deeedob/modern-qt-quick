include_guard(GLOBAL)

function(app_archive_symbols target)
    set(symbolDirectory "${PROJECT_BINARY_DIR}/symbols/$<CONFIG>")
    if(APPLE)
        find_program(APP_DSYMUTIL dsymutil REQUIRED)
        if(NOT CMAKE_STRIP)
            message(FATAL_ERROR "Release symbol archival requires strip.")
        endif()
        set(installedBinary "${PROJECT_NAME}.app/Contents/MacOS/$<TARGET_FILE_NAME:${target}>")
        set(commands [[
            execute_process(COMMAND "@APP_DSYMUTIL@" "$<TARGET_FILE:@target@>"
                -o "@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.dSYM"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_STRIP@" -S "${installedBinary}"
                COMMAND_ERROR_IS_FATAL ANY)
        ]])
    elseif(MSVC)
        set(commands [[
            file(COPY "$<TARGET_PDB_FILE:@target@>" DESTINATION "@symbolDirectory@")
        ]])
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(NOT CMAKE_OBJCOPY OR NOT CMAKE_STRIP)
            message(FATAL_ERROR "Release symbol archival requires objcopy and strip.")
        endif()
        set(installedBinary "${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:${target}>")
        set(commands [[
            execute_process(COMMAND "@CMAKE_OBJCOPY@" --only-keep-debug
                "$<TARGET_FILE:@target@>" "@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.debug"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_STRIP@" --strip-debug "${installedBinary}"
                COMMAND_ERROR_IS_FATAL ANY)
            execute_process(COMMAND "@CMAKE_OBJCOPY@"
                "--add-gnu-debuglink=@symbolDirectory@/$<TARGET_FILE_NAME:@target@>.debug"
                "${installedBinary}" COMMAND_ERROR_IS_FATAL ANY)
        ]])
    else()
        set(commands [[
            message(FATAL_ERROR "Symbol archival supports macOS, MSVC and Linux toolchains.")
        ]])
    endif()
    string(CONFIGURE "${commands}" commands @ONLY)
    set(script [[
        if(CMAKE_INSTALL_CONFIG_NAME STREQUAL "RelWithDebInfo")
            set(installedBinary "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/@installedBinary@")
            file(MAKE_DIRECTORY "@symbolDirectory@")
            @commands@
            file(WRITE "@symbolDirectory@/build.txt"
                "@PROJECT_NAME@ @PROJECT_VERSION@\ncommit=@APP_GIT_SHA@\nplatform=@CMAKE_SYSTEM_NAME@\narchitecture=@CMAKE_SYSTEM_PROCESSOR@\nconfiguration=$<CONFIG>\n")
        endif()
    ]])
    string(CONFIGURE "${script}" script @ONLY)
    install(CODE "${script}")
endfunction()
