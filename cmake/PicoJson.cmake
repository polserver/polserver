set(PICO_REPO "https://github.com/kazuho/picojson")
set(PICO_TAG "111c9be")

set(PICO_SOURCE_DIR "${POL_EXT_LIB_DIR}/picojson-${PICO_TAG}")

message("* libpicojson")
add_library(libpicojson INTERFACE IMPORTED)

if (NOT EXISTS "${PICO_SOURCE_DIR}")
  ExternalProject_Add(libpicojson_ext
    GIT_REPOSITORY   ${PICO_REPO}
    GIT_TAG          ${PICO_TAG}
    SOURCE_DIR  ${PICO_SOURCE_DIR}/picojson
    PREFIX pico
    
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""

    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
    EXCLUDE_FROM_ALL 1
  )
else()
  message("  - already downloaded")
endif()
set_target_properties(libpicojson PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PICO_SOURCE_DIR}"
)
add_dependencies(libpicojson libpicojson_ext)
