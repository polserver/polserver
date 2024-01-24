# https://github.com/fmtlib/fmt

message("* format")

set(FMT_SOURCE_DIR "${POL_EXT_LIB_DIR}/fmt-10.2.0")
set(FMT_INSTALL_DIR "${FMT_SOURCE_DIR}/install")
set(FMT_ARGS -DCMAKE_BUILD_TYPE=Release
   -DCMAKE_INSTALL_PREFIX=${FMT_INSTALL_DIR}
   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
   -DBUILD_SHARED_LIBS=Off
   -DFMT_MASTER_PROJECT=Off # no docs/tests
   -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_CURRENT_LIST_DIR}/cxx_flag_overrides.cmake
   -DCMAKE_USER_MAKE_RULES_OVERRIDE_C=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake
   -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded # /Mt instead lf /Md
   -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
 )
if (${linux})
  set(FMT_LIB "${FMT_INSTALL_DIR}/lib/libfmt.a")
else()
  set(FMT_LIB "${FMT_INSTALL_DIR}/lib/fmt.lib")
endif()

ExternalProject_Add(fmt_Ext
  SOURCE_DIR  ${FMT_SOURCE_DIR}
    PREFIX fmt
    LIST_SEPARATOR |
    CMAKE_ARGS ${FMT_ARGS}
    BINARY_DIR ${FMT_SOURCE_DIR}/build
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install

    BUILD_BYPRODUCTS ${FMT_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
  )

# imported target to add include/lib dir and additional dependencies
add_library(libfmt STATIC IMPORTED)
set_target_properties(libfmt PROPERTIES
  IMPORTED_LOCATION ${FMT_LIB}
  IMPORTED_IMPLIB ${FMT_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${FMT_INSTALL_DIR}/include
  FOLDER 3rdParty
)
file(MAKE_DIRECTORY ${FMT_INSTALL_DIR}/include) #directory has to exist during configure

add_dependencies(libfmt fmt_Ext)
