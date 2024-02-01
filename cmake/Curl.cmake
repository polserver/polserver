message("* libcurl")
set(CURL_SOURCE_DIR "${POL_EXT_LIB_DIR}/curl-8.2.1")

set(CURL_FLAGS -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF -DCURL_STATICLIB=ON -DBUILD_SHARED_LIBS=OFF -DCURL_DISABLE_LDAP=ON -DUSE_LIBIDN2=OFF)
if (${linux})
  include(GNUInstallDirs)
  set(CURL_INSTALL_DIR "${CURL_SOURCE_DIR}/INSTALL")
  set(CURL_LIB "${CURL_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libcurl.a")
else()
  set(CURL_INSTALL_DIR "${CURL_SOURCE_DIR}/builds/libcurl-${ARCH_STRING}-release-static")
  set(CURL_LIB "${CURL_INSTALL_DIR}/lib/libcurl.lib")
  set(CURL_FLAGS ${CURL_FLAGS} -DCURL_USE_SCHANNEL=ON -DCURL_STATIC_CRT=1 -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake)
endif()

if(NOT EXISTS "${CURL_LIB}")
  ExternalProject_Add(libcurl_ext
    URL "${CURL_SOURCE_DIR}/../curl-8.2.1.zip"
    SOURCE_DIR  "${CURL_SOURCE_DIR}"
    PREFIX curl
    LIST_SEPARATOR |
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CURL_INSTALL_DIR} ${CURL_FLAGS} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_IN_SOURCE 1
    BUILD_BYPRODUCTS ${CURL_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
    DOWNLOAD_EXTRACT_TIMESTAMP 1
    EXCLUDE_FROM_ALL 1
  )
  set_target_properties (libcurl_ext PROPERTIES FOLDER 3rdParty)
  file(MAKE_DIRECTORY ${CURL_INSTALL_DIR}/include) #directory has to exist during configure
else()
  message("  - already build")
endif()

# imported target to add include/lib dir and additional dependencies
add_library(libcurl STATIC IMPORTED)
set_target_properties(libcurl PROPERTIES
  IMPORTED_LOCATION ${CURL_LIB}
  IMPORTED_IMPLIB ${CURL_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${CURL_INSTALL_DIR}/include
  INTERFACE_COMPILE_DEFINITIONS CURL_STATICLIB
  FOLDER 3rdParty
)
if (${linux})
  set_property(TARGET libcurl 
    PROPERTY INTERFACE_LINK_LIBRARIES ssl)
  if (APPLE)
    pkg_search_module(LIBSSH2 REQUIRED libssh2 IMPORTED_TARGET)
    if(TARGET PkgConfig::LIBSSH2)
      set_property(TARGET libcurl APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES PkgConfig::LIBSSH2)
    endif()
    find_library(CoreFoundation_Library CoreFoundation)
    find_library(CoreServices_Library CoreServices)
    find_library(SystemConfiguration_Library SystemConfiguration)
    set_property(TARGET libcurl APPEND
      PROPERTY INTERFACE_LINK_LIBRARIES 
        ${CoreFoundation_Library} ${CoreServices_Library} ${SystemConfiguration_Library})
  endif()
else()
  set_property(TARGET libcurl 
    PROPERTY INTERFACE_LINK_LIBRARIES wldap32)
endif()
add_dependencies(libcurl libcurl_ext)
