# windows only: build zlib

message("* zlib")
set(ZLIB_SOURCE_DIR "${POL_EXT_LIB_DIR}/zlib-1.3")

set(ZLIB_INSTALL_DIR "${ZLIB_SOURCE_DIR}/builds")
set(ZLIB_LIB "${ZLIB_INSTALL_DIR}/lib/zlibstatic.lib")
set(ZLIB_FLAGS -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake)

if(NOT EXISTS "${ZLIB_LIB}")
  ExternalProject_Add(libz
    URL "${ZLIB_SOURCE_DIR}/../zlib13.zip"
    SOURCE_DIR  "${ZLIB_SOURCE_DIR}"
    PREFIX z
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR} ${ZLIB_FLAGS} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_IN_SOURCE 1
    BUILD_BYPRODUCTS ${ZLIB_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
  )
  set_target_properties (libz PROPERTIES FOLDER 3rdParty)
else()
  message("Zlib already build")
endif()
