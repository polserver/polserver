# To update the ANTLR library:
# 1. Download an archive from the GitHub Releases page: https://github.com/antlr/antlr4/releases
#    - Example URL: https://github.com/antlr/antlr4/archive/4.8.tar.gz
# 2. Copy the contents of runtime/Cpp into lib/antlr
# 3. Replace all instances of ERROR with ANTLR_ERROR
#    - This is because windows headers define a macro named ERROR,
#      and precompiled headers pull that macro in.
# 4. Remove in runtime/CMakeList.txt the build of "antlr4_shared"
# 5. Change in runtime/CMakeList.txt:
# if(WIN32) #<-- this
#  if(WITH_STATIC_CRT)
#    target_compile_options(antlr4_static PRIVATE "/MT$<$<CONFIG:Debug>:d>")

message("* antlr")
set(ANTLR_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/antlr")
set(ANTLR_INSTALL_DIR "${ANTLR_SOURCE_DIR}/install")
set(ANTLR_INCLUDE_DIR "${ANTLR_INSTALL_DIR}/include/antlr4-runtime")
if (${linux})
  set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/libantlr4-runtime.a")
else()
  if ("${CMAKE_GENERATOR}" MATCHES "Ninja" AND ${clang})
    set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/antlr4-runtime.lib")
  else()
    set(ANTLR_LIB "${ANTLR_INSTALL_DIR}/lib/antlr4-runtime-static.lib")
  endif()
endif()
if (NOT EXISTS ${ANTLR_LIB})
  ExternalProject_Add(libantlr_ex
    SOURCE_DIR  ${ANTLR_SOURCE_DIR}
    PREFIX antlr
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${ANTLR_INSTALL_DIR} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DWITH_LIBCXX=Off
    BINARY_DIR ${ANTLR_SOURCE_DIR}/build
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
 
    BUILD_BYPRODUCTS ${ANTLR_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
  )
else()
  message("Antlr already build")
endif()
