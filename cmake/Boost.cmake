message("* boost")
set (BOOST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/boost_1_67_0")

if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
  # build disabled no need currenty
  ExternalProject_Add(boost
	URL "${BOOST_SOURCE_DIR}/../boost_1_67_0.tar.bz2"
    SOURCE_DIR "${BOOST_SOURCE_DIR}" 
#	CONFIGURE_COMMAND ./bootstrap.sh
#       --prefix=build
	INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
#	BUILD_COMMAND ./bjam
#       toolset=gcc
#       link=shared
#       runtime-link=shared
#       address-model=${ARCH_BITS}
#       variant=release
#       --abbreviate-paths
#       --stagedir=lib
#       --build-dir=build
#       --with-regex
#       stage
	LOG_DOWNLOAD 1
    BUILD_IN_SOURCE 1
  )
else()
  message("Boost already extracted")
endif()

set (BOOST_LIBRARY_DIR "${BOOST_SOURCE_DIR}/lib/lib")

