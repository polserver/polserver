message("* boost")
set (BOOST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/boost_1_67_0")
set (BOOST_STAGE_LIB_DIR "${BOOST_SOURCE_DIR}/stage/lib")

if (clang)
  set (BOOST_TOOLSET "clang")
elseif (gcc)
  set (BOOST_TOOLSET "gcc")
elseif (msvc)
  if (MSVC_TOOLSET_VERSION EQUAL 140)
    set (BOOST_TOOLSET "msvc-14.0") # VS 2015 (14.0)
    set (BOOST_LIB_TAG "vc140-mt")
  elseif(MSVC_TOOLSET_VERSION GREATER_EQUAL 141)
    set(BOOST_TOOLSET "msvc-14.1") # VS 2017 (15.0) and VS 2019 (16.0)
    set (BOOST_LIB_TAG "vc141-mt")
  endif()
endif()

if (NOT DEFINED BOOST_TOOLSET)
  message(FATAL_ERROR "Unknown boost toolset to build")
endif()

if (${windows})
  set (BOOST_CONFIGURE_COMMAND "bootstrap.bat")
  set (BOOST_BUILD_COMMAND "b2.exe")
  if (debug)
    set (BOOST_LIB_TAG "${BOOST_LIB_TAG}-s") # The Debug configuration still links with this the non-'sgd' static build?
  else()
    set (BOOST_LIB_TAG "${BOOST_LIB_TAG}-s")
  endif()
  set (BOOST_FILESYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_filesystem-${BOOST_LIB_TAG}-${ARCH_STRING}-1_67.lib" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system-${BOOST_LIB_TAG}-${ARCH_STRING}-1_67.lib" )
else()  
  set (BOOST_CONFIGURE_COMMAND "./bootstrap.sh")
  set (BOOST_BUILD_COMMAND "./b2")
  set (BOOST_TOOLSET "gcc")
  set (BOOST_FILESYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_filesystem.a" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.a" )
endif()

if(debug)
  set (BOOST_BUILD_VARIANT "release") # Apparently our debug build still links with boost release libs
else()
  set (BOOST_BUILD_VARIANT "release")
endif()

if(NOT EXISTS ${BOOST_FILESYSTEM_LIB} OR NOT EXISTS ${BOOST_SYSTEM_LIB})
  message("  - will build in ${BOOST_SOURCE_DIR}")
  ExternalProject_Add(boost
    SOURCE_DIR ${BOOST_SOURCE_DIR}
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    DOWNLOAD_COMMAND ""
    BUILD_COMMAND ${BOOST_BUILD_COMMAND} address-model=${ARCH_BITS} toolset=${BOOST_TOOLSET} variant=${BOOST_BUILD_VARIANT} link=static runtime-link=static --with-filesystem stage
    BUILD_BYPRODUCTS ${BOOST_FILESYSTEM_LIB} ${BOOST_SYSTEM_LIB}
    LOG_BUILD 1
    BUILD_IN_SOURCE 1
    LOG_OUTPUT_ON_FAILURE 1
  )
  set_target_properties (boost PROPERTIES FOLDER 3rdParty)
  if (NOT EXISTS ${BOOST_STAGE_LIB_DIR})
    message("  - will extract")
    ExternalProject_Add(boost_extract
      URL "${BOOST_SOURCE_DIR}/../boost_1_67_0.tar.bz2"
      SOURCE_DIR "${BOOST_SOURCE_DIR}"
      CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND}
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

