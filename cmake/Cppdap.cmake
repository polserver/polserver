message("* cppdap")

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
  ExternalProject_Add(cppdap_Ext
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
  )
endif()

add_library(cppdap STATIC IMPORTED)
set_target_properties(cppdap PROPERTIES IMPORTED_LOCATION ${CPPDAP_LIB})
set_target_properties(cppdap PROPERTIES IMPORTED_IMPLIB ${CPPDAP_LIB})

file(MAKE_DIRECTORY ${CPPDAP_INSTALL_DIR}/include) #directory has to exist during configure
set_target_properties(cppdap PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CPPDAP_INSTALL_DIR}/include)

if(NOT EXISTS ${CPPDAP_LIB})
  add_dependencies(cppdap cppdap_Ext)
  set_target_properties (cppdap PROPERTIES FOLDER 3rdParty)
endif()
