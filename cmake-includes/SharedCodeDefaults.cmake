if (MSVC)
    # fast math and better simd support in RELEASE
    # https://learn.microsoft.com/en-us/cpp/build/reference/fp-specify-floating-point-behavior?view=msvc-170#fast
    target_compile_options(SharedCode INTERFACE $<$<CONFIG:RELEASE>:/fp:precise>)
else ()
    # See the implications here:
    # https://stackoverflow.com/q/45685487
    if (WIN32)
        target_compile_options(SharedCode INTERFACE $<$<CONFIG:RELEASE>:-O3 /fp:precise -fno-signed-zeros -freciprocal-math>)
        target_compile_options(SharedCode INTERFACE $<$<CONFIG:RelWithDebInfo>:-O3 /fp:precise -fno-signed-zeros -freciprocal-math>)
    else()
        target_compile_options(SharedCode INTERFACE $<$<CONFIG:RELEASE>:-O3 -ffp-contract=fast -fno-signed-zeros -freciprocal-math>)
        target_compile_options(SharedCode INTERFACE $<$<CONFIG:RelWithDebInfo>:-O3 -ffp-contract=fast -fno-signed-zeros -freciprocal-math>)
    endif ()
endif ()

# Tell MSVC to properly report what c++ version is being used
if (MSVC)
    target_compile_options(SharedCode INTERFACE /Zc:__cplusplus)
endif ()

# C++20, please
# Use cxx_std_23 for C++23 (as of CMake v 3.20)
target_compile_features(SharedCode INTERFACE cxx_std_20)