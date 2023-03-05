message("* cppdap")

set (CPPDAP_VERSION_REF "59819690ec4114b01aae73b4caf22650f029ce53")
set (CPPDAP_SOURCE_DIR "${POL_EXT_LIB_DIR}/cppdap-${CPPDAP_VERSION_REF}")

if (${windows})
  set (CPPDAP_LIB "${CPPDAP_SOURCE_DIR}/libcppdap.a.lib" )
else()
  set (CPPDAP_LIB "${CPPDAP_SOURCE_DIR}/libcppdap.a" )
endif()

if(NOT EXISTS ${CPPDAP_LIB})
  ExternalProject_Add(cppdap
    GIT_REPOSITORY  https://github.com/google/cppdap.git
    GIT_TAG         ${CPPDAP_VERSION_REF}
    GIT_SHALLOW     TRUE
    STEP_TARGETS    build
    SOURCE_DIR      "${CPPDAP_SOURCE_DIR}"
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${CPPDAP_LIB}"
  )
else()
  message("  cppdap already built")
endif()
