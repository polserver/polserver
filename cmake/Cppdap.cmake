message("* libcppdap")

set (CPPDAP_REPOSITORY "https://github.com/KevinEady/cppdap.git")
set (CPPDAP_VERSION_REF "2a4c7cf")

set (CPPDAP_SOURCE_DIR "${POL_EXT_LIB_DIR}/cppdap-${CPPDAP_VERSION_REF}")
set (CPPDAP_INSTALL_DIR "${CPPDAP_SOURCE_DIR}/build")

if (${windows})
  set (CPPDAP_LIB "${CPPDAP_INSTALL_DIR}/lib/cppdap.lib" )
else()
  set (CPPDAP_LIB "${CPPDAP_INSTALL_DIR}/lib/libcppdap.a" )
endif()

if (${windows})
  set(CPPDAP_FLAGS -DCMAKE_CXX_FLAGS_RELEASE="/MT" -DCMAKE_C_FLAGS_RELEASE="/MT")
endif()

set(CPPDAP_ARGS ${CPPDAP_FLAGS}
  -DCMAKE_INSTALL_PREFIX=${CPPDAP_INSTALL_DIR}
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
 )

if(NOT EXISTS ${CPPDAP_LIB})
  ExternalProject_Add(libcppdap_ext
    GIT_REPOSITORY   ${CPPDAP_REPOSITORY}
    GIT_TAG          ${CPPDAP_VERSION_REF}
    GIT_SHALLOW      TRUE
    PREFIX           cppdap
    LIST_SEPARATOR |
    CMAKE_ARGS       ${CPPDAP_ARGS}
    SOURCE_DIR       "${CPPDAP_SOURCE_DIR}"
    BUILD_IN_SOURCE  1
    INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_BYPRODUCTS ${CPPDAP_LIB}
    EXCLUDE_FROM_ALL 1
  )
else()
  message("  - already built")
endif()

add_library(libcppdap STATIC IMPORTED)
set_target_properties(libcppdap PROPERTIES
  IMPORTED_LOCATION ${CPPDAP_LIB}
  IMPORTED_IMPLIB ${CPPDAP_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${CPPDAP_INSTALL_DIR}/include
  FOLDER 3rdParty
)
if(NOT EXISTS ${CPPDAP_LIB})
  file(MAKE_DIRECTORY ${CPPDAP_INSTALL_DIR}/include) #directory has to exist during configure
  add_dependencies(libcppdap libcppdap_ext)
endif()
