# To update the ANTLR library:
# 1. Download an archive from the GitHub Releases page: https://github.com/antlr/antlr4/releases
#    - Example URL: https://github.com/antlr/antlr4/archive/4.8.tar.gz
# 2. Copy the contents of runtime/Cpp into lib/antlr
# 3. regenerate EScriptGrammar
# 4. Add to runtime/CMakeList.txt in the `if(CMAKE_SYSTEM_NAME MATCHES "Linux")` block:
#   target_compile_options(antlr4_static PRIVATE "-fPIC")

message("* libantlr")
set(ANTLR_SOURCE_DIR "${POL_EXT_LIB_DIR}/antlr")
set(ANTLR_INSTALL_DIR "${ANTLR_SOURCE_DIR}/install")
set(ANTLR_INCLUDE_DIR "${ANTLR_INSTALL_DIR}/include/antlr4-runtime")
if (${linux})
  set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/libantlr4-runtime.a")
else()
  set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/antlr4-runtime-static.lib")
endif()

if (NOT EXISTS ${ANTLR_LIB})
  ExternalProject_Add(libantlr_ext
    SOURCE_DIR  ${ANTLR_SOURCE_DIR}
    PREFIX antlr
    LIST_SEPARATOR |
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
    BINARY_DIR ${ANTLR_SOURCE_DIR}/build
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
