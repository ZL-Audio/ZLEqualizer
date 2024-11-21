# Enable Sanitizer
if (DEFINED ENV{SANITIZER_FLAG})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    add_compile_options("-fsanitize=address")
    link_libraries("-fsanitize=address")
    message("Enable Address Sanitizer")
endif ()