message("* libantlr")
set(ANTLR_REPOSITORY "https://github.com/antlr/antlr4")
# This ref points to a commit on the `dev` branch, as it contains `#include`
# directives missing for the NOPCH build.
set(ANTLR_VERSION_REF "cb85012")

set(ANTLR_SOURCE_DIR "${POL_EXT_LIB_DIR}/antlr-${ANTLR_VERSION_REF}")
set(ANTLR_INSTALL_DIR "${ANTLR_SOURCE_DIR}/install")
set(ANTLR_INCLUDE_DIR "${ANTLR_INSTALL_DIR}/include/antlr4-runtime")
set(ANTLR_BUILD_DIR "${ANTLR_SOURCE_DIR}/build")

if (${linux})
  set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/libantlr4-runtime.a")
else()
  set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/antlr4-runtime-static.lib")
endif()

if (NOT EXISTS ${ANTLR_LIB})
  ExternalProject_Add(libantlr_ext
    GIT_REPOSITORY   ${ANTLR_REPOSITORY}
    GIT_TAG          ${ANTLR_VERSION_REF}
    SOURCE_DIR  ${ANTLR_SOURCE_DIR}
    PREFIX antlr
    LIST_SEPARATOR |
    SOURCE_SUBDIR runtime/Cpp
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_INSTALL_PREFIX=${ANTLR_INSTALL_DIR}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DWITH_LIBCXX=Off
      -DANTLR_BUILD_SHARED=Off
      -DANTLR_BUILD_CPP_TESTS=Off
      -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install

    BUILD_BYPRODUCTS ${ANTLR_LIB}
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

# imported target to add include/lib dir and additional dependencies
add_library(libantlr STATIC IMPORTED)
set_target_properties(libantlr PROPERTIES
  IMPORTED_LOCATION ${ANTLR_LIB}
  IMPORTED_IMPLIB ${ANTLR_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${ANTLR_INCLUDE_DIR}
  INTERFACE_COMPILE_DEFINITIONS ANTLR4CPP_STATIC
  FOLDER 3rdParty
)
if (NOT EXISTS ${ANTLR_LIB})
  file(MAKE_DIRECTORY ${ANTLR_INCLUDE_DIR}) #directory has to exist during configure
  add_dependencies(libantlr libantlr_ext)
endif()
