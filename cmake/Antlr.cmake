
message("* antlr")
set(ANTLR_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/antlr")
set(ANTLR_INSTALL_DIR "${ANTLR_SOURCE_DIR}/install")
set(ANTLR_INCLUDE_DIR "${ANTLR_INSTALL_DIR}/include/antlr4-runtime")
if (${linux})
  set(ANTLR_LIB "${ANTLR_SOURCE_DIR}/dist/libantlr4-runtime.a")
else()
  set(ANTLR_LIB "${ANTLR_SOURCE_DIR}/dist/libantlr4-runtime-static.lib")
endif()
if (NOT EXISTS ${ANTLR_LIB})
  ExternalProject_Add(libantlr_ex
    SOURCE_DIR  ${ANTLR_SOURCE_DIR}
    PREFIX antlr
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${ANTLR_INSTALL_DIR} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DWITH_LIBCXX=Off
    BINARY_DIR ${ANTLR_SOURCE_DIR}/build
    BUILD_BYPRODUCTS ${ANTLR_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
  )
else()
  message("Antlr already build")
endif()
