target_compile_features(SharedCode INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(SharedCode INTERFACE
            "$<$<CONFIG:Release>:/fp:precise>"
            /Zc:__cplusplus
    )
else ()
    # Apply the same floating-point optimizations to both release configurations.
    target_compile_options(SharedCode INTERFACE
            "$<$<CONFIG:Release,RelWithDebInfo>:-O3;-ffp-contract=fast;-fno-signed-zeros;-freciprocal-math>"
    )
endif ()
