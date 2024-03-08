message("* libefsw")

set (EFSW_REPOSITORY "https://github.com/SpartanJ/efsw.git")
set (EFSW_VERSION_REF "3419347")

set (EFSW_SOURCE_DIR "${POL_EXT_LIB_DIR}/efsw-${EFSW_VERSION_REF}")
set (EFSW_INSTALL_DIR "${EFSW_SOURCE_DIR}/build")

if (${windows})
  set (EFSW_LIB "${EFSW_INSTALL_DIR}/lib/efsw-static.lib" )
else()
  set (EFSW_LIB "${EFSW_INSTALL_DIR}/lib/libefsw-static.a" )
endif()

set(EFSW_ARGS ${EFSW_FLAGS}
  -DCMAKE_INSTALL_PREFIX=${EFSW_INSTALL_DIR}
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
  -DBUILD_TEST_APP=OFF
  -DBUILD_SHARED_LIBS=ON # The -static file isn't installed unless this is enabled?
  -DBUILD_STATIC_LIBS=ON
  -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_CURRENT_LIST_DIR}/cxx_flag_overrides.cmake
  -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
 )

if(NOT EXISTS ${EFSW_LIB})
  ExternalProject_Add(libefsw_ext
    GIT_REPOSITORY   ${EFSW_REPOSITORY}
    GIT_TAG          ${EFSW_VERSION_REF}
    GIT_SHALLOW      TRUE
    PREFIX           efsw
    LIST_SEPARATOR |
    CMAKE_ARGS       ${EFSW_ARGS}
    SOURCE_DIR       "${EFSW_SOURCE_DIR}"
    BUILD_IN_SOURCE  1
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_BYPRODUCTS ${EFSW_LIB}
    EXCLUDE_FROM_ALL 1
  )
else()
  message("  - already built")
endif()

add_library(libefsw STATIC IMPORTED)
set_target_properties(libefsw PROPERTIES
  IMPORTED_LOCATION ${EFSW_LIB}
  IMPORTED_IMPLIB ${EFSW_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${EFSW_INSTALL_DIR}/include
  FOLDER 3rdParty
)
if(APPLE)
  find_library(CoreFoundation_Library CoreFoundation)
  find_library(CoreServices_Library CoreServices)
  set_property(TARGET libefsw APPEND
    PROPERTY INTERFACE_LINK_LIBRARIES
      ${CoreFoundation_Library} ${CoreServices_Library})
endif()

if(NOT EXISTS ${EFSW_LIB})
  file(MAKE_DIRECTORY ${EFSW_INSTALL_DIR}/include) #directory has to exist during configure
  add_dependencies(libefsw libefsw_ext)
endif()
