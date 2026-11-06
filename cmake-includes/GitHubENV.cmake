# Export metadata and artifact paths for the CI packaging steps.
if (NOT DEFINED ENV{CI})
    return()
endif ()

set(env_file "${PROJECT_SOURCE_DIR}/.env")
message(STATUS "Writing ENV file for CI: ${env_file}")

# Start with a fresh file on each configure.
file(WRITE "${env_file}"
        "PROJECT_NAME=${PROJECT_NAME}\n"
        "PRODUCT_NAME=${PRODUCT_NAME}\n"
        "VERSION=${CURRENT_VERSION}\n"
        "BUNDLE_ID=${BUNDLE_ID}\n"
        "COMPANY_NAME=${COMPANY_NAME}\n"
)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(APPEND "${env_file}" "MACOS_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}\n")
endif ()

file(RELATIVE_PATH RELATIVE_BINARY_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")
set(ARTIFACTS_PATH "${RELATIVE_BINARY_PATH}/${CMAKE_PROJECT_NAME}_artefacts/${CMAKE_BUILD_TYPE}")
file(APPEND "${env_file}" "ARTIFACTS_PATH=${ARTIFACTS_PATH}\n")

if ("VST3" IN_LIST FORMATS)
    file(APPEND "${env_file}" "VST3_PATH=${ARTIFACTS_PATH}/VST3/${PRODUCT_NAME}.vst3\n")
endif ()
if ("AU" IN_LIST FORMATS)
    file(APPEND "${env_file}" "AU_PATH=${ARTIFACTS_PATH}/AU/${PRODUCT_NAME}.component\n")
endif ()
if ("AAX" IN_LIST FORMATS)
    file(APPEND "${env_file}" "AAX_PATH=${ARTIFACTS_PATH}/AAX/${PRODUCT_NAME}.aaxplugin\n")
endif ()
if ("LV2" IN_LIST FORMATS)
    file(APPEND "${env_file}" "LV2_PATH=${ARTIFACTS_PATH}/LV2/${PRODUCT_NAME}.lv2\n")
endif ()
if ("Standalone" IN_LIST FORMATS)
    set(standalone_path "${ARTIFACTS_PATH}/Standalone/${PRODUCT_NAME}")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        string(APPEND standalone_path ".app")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        string(APPEND standalone_path ".exe")
    endif ()
    file(APPEND "${env_file}" "Standalone_PATH=${standalone_path}\n")

    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        get_target_property(standalone_entitlements "${PROJECT_NAME}_Standalone"
                XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS
        )
        file(APPEND "${env_file}" "Standalone_ENTITLEMENTS_PATH=${standalone_entitlements}\n")
    endif ()
endif ()
