message("* boost")
set (BOOST_SOURCE_DIR "${POL_EXT_LIB_DIR}/boost_1_74_0")
set (BOOST_STAGE_LIB_DIR "${BOOST_SOURCE_DIR}/stage/lib")

if (clang)
  set (BOOST_TOOLSET "clang")
elseif (gcc)
  set (BOOST_TOOLSET "gcc")
elseif (msvc)
  if (MSVC_TOOLSET_VERSION EQUAL 140)
    set (BOOST_TOOLSET "msvc") # VS 2015 (14.0)
  elseif(MSVC_TOOLSET_VERSION GREATER_EQUAL 141)
    set(BOOST_TOOLSET "msvc-14.1") # VS 2017 (15.0) and VS 2019 (16.0)
  endif()
endif()

if (NOT DEFINED BOOST_TOOLSET)
  message(FATAL_ERROR "Unknown boost toolset to build")
endif()

if (${windows})
  set (BOOST_CONFIGURE_COMMAND "bootstrap.bat")
  set (BOOST_BUILD_COMMAND "b2.exe")
  set (BOOST_REGEX_LIB "${BOOST_STAGE_LIB_DIR}/libboost_regex.lib" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.lib" )
  set (BOOST_THREAD_LIB "${BOOST_STAGE_LIB_DIR}/libboost_thread.lib" )
else()
  set (BOOST_CONFIGURE_COMMAND "./bootstrap.sh")
  set (BOOST_BUILD_COMMAND "./b2")
  set (BOOST_REGEX_LIB "${BOOST_STAGE_LIB_DIR}/libboost_regex.a" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.a" )
  set (BOOST_THREAD_LIB "${BOOST_STAGE_LIB_DIR}/libboost_thread.a" )
endif()

if(NOT EXISTS ${BOOST_REGEX_LIB} OR NOT EXISTS ${BOOST_SYSTEM_LIB} OR NOT EXISTS ${BOOST_THREAD_LIB})
  message("  - will build in ${BOOST_SOURCE_DIR} toolset=${BOOST_TOOLSET}")
  ExternalProject_Add(boost
          SOURCE_DIR ${BOOST_SOURCE_DIR}
          INSTALL_COMMAND ""
          CONFIGURE_COMMAND ""
          DOWNLOAD_COMMAND ""
          BUILD_COMMAND ${BOOST_BUILD_COMMAND} address-model=${ARCH_BITS} toolset=${BOOST_TOOLSET} variant=release link=static runtime-link=static --layout=system --with-regex --with-system --with-thread stage
          BUILD_BYPRODUCTS ${BOOST_REGEX_LIB} ${BOOST_SYSTEM_LIB} ${BOOST_THREAD_LIB}
          LOG_BUILD 1
          BUILD_IN_SOURCE 1
          LOG_OUTPUT_ON_FAILURE 1
          )
  set_target_properties (boost PROPERTIES FOLDER 3rdParty)
  if (NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
    message("  - will extract")
    ExternalProject_Add(boost_extract
            URL "https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.bz2"
            SOURCE_DIR "${BOOST_SOURCE_DIR}"
            CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND} --with-toolset=${BOOST_TOOLSET}
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            BUILD_BYPRODUCTS "${BOOST_SOURCE_DIR}"
            LOG_DOWNLOAD 1
            LOG_CONFIGURE 1
            BUILD_IN_SOURCE 1
            LOG_OUTPUT_ON_FAILURE 1
            )
    set_target_properties (boost_extract PROPERTIES FOLDER 3rdParty)

    add_dependencies(boost boost_extract)
  else()
    message("  - will not extract")
  endif()
else()
  message("  - will not build")
endif()
