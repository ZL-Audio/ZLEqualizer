target_compile_features(SharedCode INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(SharedCode INTERFACE
            "$<$<CONFIG:Release>:/fp:precise>"
            /Zc:__cplusplus
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(SharedCode INTERFACE
                "$<$<CONFIG:Release,RelWithDebInfo>:/clang:-ffp-contract=fast-honor-pragmas>"
        )
    endif ()
else ()
    target_compile_options(SharedCode INTERFACE
            "$<$<CONFIG:Release,RelWithDebInfo>:-O3;-fno-signed-zeros;-freciprocal-math>"
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        target_compile_options(SharedCode INTERFACE
                "$<$<CONFIG:Release,RelWithDebInfo>:-ffp-contract=fast-honor-pragmas>"
        )
    else ()
        target_compile_options(SharedCode INTERFACE
                "$<$<CONFIG:Release,RelWithDebInfo>:-ffp-contract=fast>"
        )
    endif ()
endif ()
