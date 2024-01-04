# https://github.com/kaitai-io/kaitai_struct_cpp_stl_runtime

message("* kaitai")

set(KAITAI_SOURCE_DIR "${POL_EXT_LIB_DIR}/kaitai-runtime")
set(KAITAI_INSTALL_DIR "${KAITAI_SOURCE_DIR}/install")
set(KAITAI_ARGS -DCMAKE_BUILD_TYPE=Release
   -DCMAKE_INSTALL_PREFIX=${KAITAI_INSTALL_DIR}
   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
   -DSTRING_ENCODING_TYPE=NONE
   -DBUILD_SHARED_LIBS=Off
   -DBUILD_TESTS=Off
   -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_CURRENT_LIST_DIR}/cxx_flag_overrides.cmake
   -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
   -DCMAKE_POLICY_DEFAULT_CMP0074=NEW # _ROOT vars can be set
   -DCMAKE_DISABLE_FIND_PACKAGE_Iconv=TRUE # we dont want to link against
 )
if (${linux})
  set(KAITAI_LIB "${KAITAI_INSTALL_DIR}/lib/libkaitai_struct_cpp_stl_runtime.a")
else()
  set(KAITAI_LIB "${KAITAI_INSTALL_DIR}/lib/kaitai_struct_cpp_stl_runtime.lib")
  set(KAITAI_ARGS ${KAITAI_ARGS}
    -DZLIB_ROOT=${ZLIB_INSTALL_DIR} # own build zlib
    )
endif()

ExternalProject_Add(kaitai_Ext
  SOURCE_DIR  ${KAITAI_SOURCE_DIR}
    PREFIX kaitai
    LIST_SEPARATOR |
    CMAKE_ARGS ${KAITAI_ARGS}
    BINARY_DIR ${KAITAI_SOURCE_DIR}/build
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install

    BUILD_BYPRODUCTS ${KAITAI_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
  )
if (${windows})
  if (NOT EXISTS "${ZLIB_LIB}")
    ExternalProject_Add_StepDependencies(kaitai_Ext configure libz)
  endif()
endif()

# imported target to add include/lib dir and additional dependencies
# dependency to the external project needs to be explicit done in the linking library
add_library(kaitai STATIC IMPORTED)
set_target_properties(kaitai PROPERTIES IMPORTED_LOCATION ${KAITAI_LIB})
set_target_properties(kaitai PROPERTIES IMPORTED_IMPLIB ${KAITAI_LIB})

file(MAKE_DIRECTORY ${KAITAI_INSTALL_DIR}/include) #directory has to exist during configure
set_target_properties(kaitai PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${KAITAI_INSTALL_DIR}/include)

if (${windows})
  set_target_properties(kaitai PROPERTIES INTERFACE_LINK_LIBRARIES ${ZLIB_LIB})
else()
  set_target_properties(kaitai PROPERTIES INTERFACE_LINK_LIBRARIES z)
endif()
add_dependencies(kaitai kaitai_Ext)
set_target_properties (kaitai PROPERTIES FOLDER 3rdParty)
