message("* cppdap")

set (CPPDAP_REPOSITORY "https://github.com/KevinEady/cppdap.git")
set (CPPDAP_VERSION_REF "2703cf4")

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

if(NOT EXISTS ${CPPDAP_LIB})
  ExternalProject_Add(cppdap
    GIT_REPOSITORY   ${CPPDAP_REPOSITORY}
    GIT_TAG          ${CPPDAP_VERSION_REF}
    GIT_SHALLOW      TRUE
    PREFIX           cppdap
    CMAKE_ARGS       ${CPPDAP_FLAGS} -DCMAKE_INSTALL_PREFIX=${CPPDAP_INSTALL_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    SOURCE_DIR       "${CPPDAP_SOURCE_DIR}"
    BUILD_IN_SOURCE  1
    INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_BYPRODUCTS ${CPPDAP_LIB}
  )
  set_target_properties (cppdap PROPERTIES FOLDER 3rdParty)
else()
  message("  cppdap already built")
endif()
