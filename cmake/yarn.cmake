message("* yarn")

set(YARN_VERSION "v1.17.3")
set(YARN_URL "https://github.com/yarnpkg/yarn/releases/download/${YARN_VERSION}/yarn-${YARN_VERSION}.tar.gz")
set(YARN_URL_MD5 "4a02e1687a150113ad6b0215f9afdb3e")

set(YARN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/yarn-${YARN_VERSION}")
set(YARN_LIB "${YARN_SOURCE_DIR}/bin/yarn.js")
  ExternalProject_Add(yarn
    URL "${YARN_URL}"
    URL_MD5 "${YARN_URL_MD5}"

    SOURCE_DIR "${YARN_SOURCE_DIR}" 
    
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    
    LOG_DOWNLOAD 1
    BUILD_IN_SOURCE 1
    BUILD_BYPRODUCTS ${YARN_LIB}
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
  )
set_target_properties (yarn PROPERTIES FOLDER 3rdParty)

message("**** YARN IS AT ${YARN_LIB} ****")