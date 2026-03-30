message("* mariadb")

set(MARIADB_REPO "https://github.com/mariadb-corporation/mariadb-connector-c.git")
set(MARIADB_TAG "v3.4.8")

if (${linux})
  find_package(ZLIB QUIET)
else()
  get_target_property(ZLIB_LIBRARIES libz IMPORTED_LOCATION)
  get_target_property(ZLIB_INCLUDE_DIRS libz INTERFACE_INCLUDE_DIRECTORIES)
endif()

set(MARIADB_SOURCE_DIR "${EXT_DOWNLOAD_DIR}/mariadb-${MARIADB_TAG}")
set(MARIADB_INSTALL_DIR "${MARIADB_SOURCE_DIR}/install")
set(MARIADB_ARGS
   -DCMAKE_BUILD_TYPE=Release
   -DCMAKE_INSTALL_PREFIX=${MARIADB_INSTALL_DIR}
   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
   -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_CURRENT_LIST_DIR}/cxx_flag_overrides.cmake
   -DCMAKE_USER_MAKE_RULES_OVERRIDE_C=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake
   -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded # /Mt instead lf /Md
   -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
   -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}

   -DWITH_UNIT_TESTS=OFF
   -DWITH_EXTERNAL_ZLIB=ON
   -DZLIB_FOUND=TRUE
   -DZLIB_LIBRARY=${ZLIB_LIBRARIES}
   -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIRS}
   -DWITH_CURL=OFF
   -DCURL_DIR=${CURL_INSTALL_DIR}/lib/cmake/CURL
   -DCLIENT_PLUGIN_CLIENT_ED25519=STATIC
   -DCLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC
   -DCLIENT_PLUGIN_SHA256_PASSWORD=STATIC
 )
if (${linux})
  include(GNUInstallDirs)
  set(MARIADB_LIB "${MARIADB_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/mariadb/libmariadbclient.a")
else()
  set(MARIADB_LIB "${MARIADB_INSTALL_DIR}/lib/mariadb/mariadbclient.lib")
endif()

if (NOT EXISTS ${MARIADB_LIB})
  ExternalProject_Add(libmaria_ext
    GIT_REPOSITORY   ${MARIADB_REPO}
    GIT_TAG          ${MARIADB_TAG}
    GIT_SHALLOW      TRUE
    SOURCE_DIR  ${MARIADB_SOURCE_DIR}
    PREFIX maria
    LIST_SEPARATOR |
    CMAKE_ARGS ${MARIADB_ARGS}
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install

    BUILD_BYPRODUCTS ${MARIADB_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    LOG_OUTPUT_ON_FAILURE 1
    EXCLUDE_FROM_ALL 1
  )

  ExternalProject_Add_StepDependencies(libmaria_ext configure libz libcurl)

else()
  message("  - already built")
endif()

# imported target to add include/lib dir and additional dependencies
add_library(libsql STATIC IMPORTED)
set_target_properties(libsql PROPERTIES
  IMPORTED_LOCATION ${MARIADB_LIB}
  IMPORTED_IMPLIB ${MARIADB_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${MARIADB_INSTALL_DIR}/include
  FOLDER 3rdParty
)
file(MAKE_DIRECTORY ${MARIADB_INSTALL_DIR}/include) #directory has to exist during configure

if(${windows})
  set_property(TARGET libsql
    PROPERTY INTERFACE_LINK_LIBRARIES secur32 crypt32 bcrypt)
endif()
add_dependencies(libsql libmaria_ext)
