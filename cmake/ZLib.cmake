# windows: build zlib
# linux: interface to z
if (${windows})
  message("* libz")
  set(ZLIB_REPO "https://github.com/madler/zlib")
  set(ZLIB_TAG "v1.3.1")
  set(ZLIB_SOURCE_DIR "${EXT_DOWNLOAD_DIR}/zlib-${ZLIB_TAG}")

  set(ZLIB_INSTALL_DIR "${ZLIB_SOURCE_DIR}/builds")
  set(ZLIB_LIB "${ZLIB_INSTALL_DIR}/lib/zlibstatic.lib")
  set(ZLIB_FLAGS -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_LIST_DIR}/c_flag_overrides.cmake)

  if(NOT EXISTS "${ZLIB_LIB}")
    ExternalProject_Add(libz_ext
      GIT_REPOSITORY   ${ZLIB_REPO}
      GIT_TAG          ${ZLIB_TAG}
      GIT_SHALLOW      TRUE
      SOURCE_DIR  "${ZLIB_SOURCE_DIR}"
      PREFIX z
      LIST_SEPARATOR |
      CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR} ${ZLIB_FLAGS} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_OSX_ARCHITECTURES=${PIPED_OSX_ARCHITECTURES}
      BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
      INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
      BUILD_IN_SOURCE 1
      DOWNLOAD_EXTRACT_TIMESTAMP 1
      BUILD_BYPRODUCTS ${ZLIB_LIB}
      LOG_DOWNLOAD 1
      LOG_CONFIGURE 1
      LOG_BUILD 1
      LOG_INSTALL 1
      LOG_OUTPUT_ON_FAILURE 1
      EXCLUDE_FROM_ALL 1
    )
    set_target_properties (libz_ext PROPERTIES FOLDER 3rdParty)
  
    file(MAKE_DIRECTORY ${ZLIB_INSTALL_DIR}/include) #directory has to exist during configure
  else()
    message("  - already build")
  endif()
  # imported target to add include/lib dir and additional dependencies
  add_library(libz STATIC IMPORTED)
  set_target_properties(libz PROPERTIES
    IMPORTED_LOCATION ${ZLIB_LIB}
    IMPORTED_IMPLIB ${ZLIB_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${ZLIB_INSTALL_DIR}/include
    FOLDER 3rdParty
  )
  add_dependencies(libz libz_ext)

else()
  add_library(libz INTERFACE IMPORTED)
  set_property(TARGET libz PROPERTY INTERFACE_LINK_LIBRARIES z)
endif()

