message("* libutf8cpp")

set(UTF_REPO "https://github.com/nemtrif/utfcpp")
set(UTF_TAG "v4.0.6")
set(UTF_SOURCE_DIR "${POL_EXT_LIB_DIR}/utf8-${UTF_TAG}")
set(UTF_INSTALL_DIR "${UTF_SOURCE_DIR}/install")

if (NOT EXISTS "${UTF_INSTALL_DIR}")
  ExternalProject_Add(libutf8_ext
    GIT_REPOSITORY   ${UTF_REPO}
    GIT_TAG          ${UTF_TAG}
    GIT_SHALLOW      TRUE
    SOURCE_DIR  ${UTF_SOURCE_DIR}
    PREFIX utf
    LIST_SEPARATOR |
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${UTF_INSTALL_DIR}
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install

    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
    EXCLUDE_FROM_ALL 1
  )
else()
  message("  - already built")
endif()
file(MAKE_DIRECTORY ${UTF_INSTALL_DIR}/include) #directory has to exist during configure

add_library(libutf8 INTERFACE IMPORTED)
set_target_properties(libutf8 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${UTF_INSTALL_DIR}/include"
)
add_dependencies(libutf8 libutf8_ext)
