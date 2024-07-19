message("* libsqlitecpp")

set (SQLITECPP_REPOSITORY "https://github.com/SRombauts/SQLiteCpp.git")
set (SQLITECPP_VERSION_REF "3.3.1")

set (SQLITECPP_SOURCE_DIR "${POL_EXT_LIB_DIR}/SQLiteCpp-${SQLITECPP_VERSION_REF}")
set (SQLITECPP_INSTALL_DIR "${SQLITECPP_SOURCE_DIR}/build")

if (${windows})
  set (SQLITECPP_LIB_1 "${SQLITECPP_INSTALL_DIR}/lib/SQLiteCpp.lib" )
  set (SQLITECPP_LIB_2 "${SQLITECPP_INSTALL_DIR}/lib/sqlite3.lib" )
else()
  set (SQLITECPP_LIB_1 "${SQLITECPP_INSTALL_DIR}/lib/libSQLiteCpp.a" )
  set (SQLITECPP_LIB_2 "${SQLITECPP_INSTALL_DIR}/lib/libsqlite3.a" )
endif()

if (${windows})
  set(SQLITECPP_FLAGS -DCMAKE_CXX_FLAGS_RELEASE="/MT" -DCMAKE_C_FLAGS_RELEASE="/MT")
endif()

set(SQLITECPP_ARGS ${SQLITECPP_FLAGS}
  -DCMAKE_INSTALL_PREFIX=${SQLITECPP_INSTALL_DIR}
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
  -DSQLITECPP_INTERNAL_SQLITE=ON
 )

if(NOT EXISTS ${SQLITECPP_LIB_1} OR NOT EXISTS ${SQLITECPP_LIB_2})
  ExternalProject_Add(libsqlitecpp_ext
    GIT_REPOSITORY   ${SQLITECPP_REPOSITORY}
    GIT_TAG          ${SQLITECPP_VERSION_REF}
    GIT_SHALLOW      TRUE
    GIT_CONFIG       core.autocrlf=false
    PREFIX           SQLiteCpp
    LIST_SEPARATOR |
    CMAKE_ARGS       ${SQLITECPP_ARGS}
    SOURCE_DIR       "${SQLITECPP_SOURCE_DIR}"
    BUILD_IN_SOURCE  1
    INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --config Release --target install
    BUILD_BYPRODUCTS ${SQLITECPP_LIB_1} ${SQLITECPP_LIB_2}
    EXCLUDE_FROM_ALL 1
  )
else()
  message("  - already built")
endif()

add_library(libsqlitecpp1 STATIC IMPORTED)
set_target_properties(libsqlitecpp1 PROPERTIES
  IMPORTED_LOCATION ${SQLITECPP_LIB_1}
  IMPORTED_IMPLIB ${SQLITECPP_LIB_1}
  INTERFACE_INCLUDE_DIRECTORIES ${SQLITECPP_INSTALL_DIR}/include
  FOLDER 3rdParty
)
add_library(libsqlitecpp2 STATIC IMPORTED)
set_target_properties(libsqlitecpp2 PROPERTIES
  IMPORTED_LOCATION ${SQLITECPP_LIB_2}
  IMPORTED_IMPLIB ${SQLITECPP_LIB_2}
  FOLDER 3rdParty
)
if(NOT EXISTS ${SQLITECPP_LIB_1})
  file(MAKE_DIRECTORY ${SQLITECPP_INSTALL_DIR}/include) #directory has to exist during configure
  add_dependencies(libsqlitecpp1 libsqlitecpp_ext)
  add_dependencies(libsqlitecpp2 libsqlitecpp_ext)
endif()
