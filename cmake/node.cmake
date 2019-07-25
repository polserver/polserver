message("* node")

set(NODE_URL_BASEPATH "https://blog.kevineady.com/files/node-static-builds/")
# set(NODE_URL_BASEPATH "${CMAKE_CURRENT_LIST_DIR}/../lib/")
set(NODE_PATHNAME_PREFIX "node-v11.12.0-")
if(${windows})
  set (NODE_DLL_NAME "node.dll")
  set (NODE_LIBRARY_NAME "node.lib")
  if (${ARCH_BITS} EQUAL "64")
    set (NODE_PATHNAME_SUFFIX "win-x64")
    Set (NODE_URL_MD5 "61546df9924c769f545176eca9ba1f21")
  endif()
else()
  set (NODE_DLL_NAME "libnode.so.67")
  set (NODE_LIBRARY_NAME ${NODE_DLL_NAME})
  if (${ARCH_BITS} EQUAL "64")
    set (NODE_PATHNAME_SUFFIX "linux-x64")
    Set (NODE_URL_MD5 "03461ede05c7272b4474250439293274")
  endif()
endif()

set(NODE_LIBRARY_DIR "Release")

set(NODE_PATHNAME "${NODE_PATHNAME_PREFIX}${NODE_PATHNAME_SUFFIX}")
set(NODE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/${NODE_PATHNAME}")
set(NODE_LIB "${NODE_SOURCE_DIR}/${NODE_LIBRARY_DIR}/${NODE_LIBRARY_NAME}")
set(NODE_DLL "${NODE_SOURCE_DIR}/${NODE_LIBRARY_DIR}/${NODE_DLL_NAME}")
set(NODE_URL "${NODE_URL_BASEPATH}${NODE_PATHNAME}.zip")

if(NOT EXISTS "${NODE_LIB}")
  message("    * extracting nodejs from ${NODE_URL} => ${NODE_SOURCE_DIR}")
    ExternalProject_Add(nodejs
      URL "${NODE_URL}"
      URL_MD5 "${NODE_URL_MD5}"

      SOURCE_DIR "${NODE_SOURCE_DIR}" 
      
      INSTALL_COMMAND ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      
      LOG_DOWNLOAD 1
      BUILD_IN_SOURCE 1
      BUILD_BYPRODUCTS ${NODE_LIB}
      LOG_CONFIGURE 1
      LOG_BUILD 1
      LOG_INSTALL 1
    )
  set_target_properties (nodejs PROPERTIES FOLDER 3rdParty)
else()
  message("node already extracted")
endif()
