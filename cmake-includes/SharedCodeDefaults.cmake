target_compile_features(SharedCode INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(SharedCode INTERFACE /Zc:__cplusplus)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(zl_clang_release_options
            -O3
            -fno-math-errno
            -fno-signed-zeros
            -freciprocal-math
            -ffp-contract=fast-honor-pragmas
    )
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        list(TRANSFORM zl_clang_release_options PREPEND "/clang:")
    endif ()
    target_compile_options(SharedCode INTERFACE "$<$<CONFIG:Release,RelWithDebInfo>:${zl_clang_release_options}>")
elseif (MSVC)
    target_compile_options(SharedCode INTERFACE "$<$<CONFIG:Release>:/fp:precise>")
else ()
    target_compile_options(SharedCode INTERFACE
            "$<$<CONFIG:Release,RelWithDebInfo>:-O3;-fno-signed-zeros;-freciprocal-math;-ffp-contract=fast>"
    )
endif ()
