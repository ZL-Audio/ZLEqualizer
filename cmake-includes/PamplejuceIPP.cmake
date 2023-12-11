# When present, use Intel IPP for performance on Windows
if (WIN32) # Can't use MSVC here, as it won't catch Clang on Windows
    find_package(IPP)
    if (IPP_FOUND)
        target_link_libraries(SharedCode INTERFACE IPP::ipps IPP::ippcore IPP::ippi IPP::ippcv)
        message("IPP LIBRARIES FOUND")
        target_compile_definitions(SharedCode INTERFACE PAMPLEJUCE_IPP=1)
    else ()
        message("IPP LIBRARIES *NOT* FOUND")
    endif ()
endif ()
