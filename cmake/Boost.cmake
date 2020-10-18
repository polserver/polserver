message("* boost")
set (BOOST_SOURCE_DIR "${POL_EXT_LIB_DIR}/boost_1_74_0")

if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
  ExternalProject_Add(boost
    URL "https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.bz2"
    SOURCE_DIR "${BOOST_SOURCE_DIR}" 
    CONFIGURE_COMMAND ./bootstrap.sh
        --prefix=build --with-toolset=gcc
    INSTALL_COMMAND ""
    BUILD_COMMAND ./bjam
        toolset=gcc
        link=static
        runtime-link=static
        address-model=${ARCH_BITS}
        variant=release
        --abbreviate-paths
        --stagedir=lib
        --build-dir=build
        --with-regex
        --with-system
        --with-thread
        stage
	LOG_DOWNLOAD 1
    BUILD_IN_SOURCE 1
  )
  set_target_properties (boost PROPERTIES FOLDER 3rdParty)
else()
  message("Boost already extracted")
endif()

set (BOOST_LIBRARY_DIR "${BOOST_SOURCE_DIR}/lib/lib")

