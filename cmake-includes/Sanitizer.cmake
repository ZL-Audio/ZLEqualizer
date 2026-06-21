option(WITH_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
option(WITH_THREAD_SANITIZER "Enable Thread Sanitizer" OFF)

message(STATUS "Sanitizers: ASan=${WITH_ADDRESS_SANITIZER} TSan=${WITH_THREAD_SANITIZER}")
if (WITH_ADDRESS_SANITIZER)
    if (MSVC)
        add_compile_options(/fsanitize=address)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            execute_process(
                    COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
                    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            add_link_options("/LIBPATH:${CLANG_RESOURCE_DIR}/lib/windows")
            link_libraries(
                    "clang_rt.asan_dynamic-x86_64.lib"
                    "clang_rt.asan_dynamic_runtime_thunk-x86_64.lib"
            )
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        link_libraries(-fsanitize=address,undefined)
    endif ()
    message(STATUS "Address Sanitizer enabled")
endif ()

if (WITH_THREAD_SANITIZER)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
        link_libraries(-fsanitize=thread)
        message(STATUS "Thread Sanitizer enabled")
    endif ()
endif ()
