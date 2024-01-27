message("* boost")
set (BOOST_SOURCE_DIR "${POL_EXT_LIB_DIR}/boost_1_83_0")
# set (BOOST_STAGE_LIB_DIR "${BOOST_SOURCE_DIR}/stage/lib")

if (clang)
  set (BOOST_TOOLSET "clang")
elseif (gcc)
  set (BOOST_TOOLSET "gcc")
elseif (msvc)
  if (MSVC_TOOLSET_VERSION EQUAL 140)
    set (BOOST_TOOLSET "msvc") # VS 2015 (14.0)
  elseif(MSVC_TOOLSET_VERSION EQUAL 141)
    set(BOOST_TOOLSET "msvc-14.1") # VS 2017 (15.0)
  elseif(MSVC_TOOLSET_VERSION EQUAL 142)
    set(BOOST_TOOLSET "msvc-14.2") # VS 2019 (16.0)
  elseif(MSVC_TOOLSET_VERSION GREATER_EQUAL 143)
    set(BOOST_TOOLSET "msvc-14.3") # VS 2022 (17.0)
  endif()
endif()

if (NOT DEFINED BOOST_TOOLSET)
  message(FATAL_ERROR "Unknown boost toolset to build")
endif()

if (${windows})
  set (BOOST_CONFIGURE_COMMAND "bootstrap.bat")
  #  set (BOOST_BUILD_COMMAND "b2.exe")
  #  set (BOOST_REGEX_LIB "${BOOST_STAGE_LIB_DIR}/libboost_regex.lib" )
  #  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.lib" )
  #  set (BOOST_THREAD_LIB "${BOOST_STAGE_LIB_DIR}/libboost_thread.lib" )
  #  if (msvc)
  #    set (BOOST_CXX_FLAGS "/MT")
  #  else()
  #    set (BOOST_CXX_FLAGS "-fms-runtime-lib=static")
  #  endif()
else()
  set (BOOST_CONFIGURE_COMMAND "./bootstrap.sh")
  #  set (BOOST_BUILD_COMMAND "./b2")
  #  set (BOOST_REGEX_LIB "${BOOST_STAGE_LIB_DIR}/libboost_regex.a" )
  #  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.a" )
  #  set (BOOST_THREAD_LIB "${BOOST_STAGE_LIB_DIR}/libboost_thread.a" )
  #
  #  set (BOOST_CXX_FLAGS "")
  #  foreach(OSX_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
  #    set (BOOST_CXX_FLAGS "${BOOST_CXX_FLAGS} -arch ${OSX_ARCHITECTURE}")
  #  endforeach()
endif()

set(boost_needs_extract FALSE)
set(boost_needs_build FALSE)
if (NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
  message("  - will extract")
  ExternalProject_Add(boost_extract
          URL "https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.tar.bz2"
          SOURCE_DIR "${BOOST_SOURCE_DIR}"
          CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND} --with-toolset=${BOOST_TOOLSET}
          BUILD_COMMAND ""
          INSTALL_COMMAND ""
          BUILD_BYPRODUCTS "${BOOST_SOURCE_DIR}"
          LOG_DOWNLOAD 1
          LOG_CONFIGURE 1
          BUILD_IN_SOURCE 1
          LOG_OUTPUT_ON_FAILURE 1
          DOWNLOAD_EXTRACT_TIMESTAMP 1
          )
  set_target_properties (boost_extract PROPERTIES FOLDER 3rdParty)
  set(boost_needs_extract TRUE)
else()
  message("  - will not extract")
endif()

#if(NOT EXISTS ${BOOST_REGEX_LIB} OR NOT EXISTS ${BOOST_SYSTEM_LIB} OR NOT EXISTS ${BOOST_THREAD_LIB})
#  message("  - will build in ${BOOST_SOURCE_DIR} toolset=${BOOST_TOOLSET} cxxflags=${BOOST_CXX_FLAGS}")
#  ExternalProject_Add(boost_build
#          SOURCE_DIR ${BOOST_SOURCE_DIR}
#          INSTALL_COMMAND ""
#          CONFIGURE_COMMAND ""
#          DOWNLOAD_COMMAND ""
#          BUILD_COMMAND ${BOOST_BUILD_COMMAND} cxxflags=${BOOST_CXX_FLAGS} address-model=${ARCH_BITS} toolset=${BOOST_TOOLSET} variant=release link=static runtime-link=static --layout=system --with-regex --with-system --with-thread stage
#          BUILD_BYPRODUCTS ${BOOST_REGEX_LIB} ${BOOST_SYSTEM_LIB} ${BOOST_THREAD_LIB}
#          LOG_BUILD 1
#          BUILD_IN_SOURCE 1
#          LOG_OUTPUT_ON_FAILURE 1
#          )
#  if (boost_needs_extract)
#    add_dependencies(boost_build boost_extract)
#  endif()
#  set_target_properties (boost_build PROPERTIES FOLDER 3rdParty)
#  set(boost_needs_build TRUE)
#else()
#  message("  - will not build")
#endif()

# imported target to add include/lib dir and additional dependencies
add_library(libboost_headers INTERFACE IMPORTED)
set_target_properties(libboost_headers PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
)
if (boost_needs_extract)
  add_dependencies(libboost_headers boost_extract)
endif()

#add_library(libboost_regex STATIC IMPORTED)
#set_target_properties(libboost_regex PROPERTIES
#  IMPORTED_LOCATION ${BOOST_REGEX_LIB}
#  IMPORTED_IMPLIB ${BOOST_REGEX_LIB}
#  INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
#  FOLDER 3rdParty
#)
#if (boost_needs_build)
#  add_dependencies(libboost_regex boost_build)
#endif()
#
#add_library(libboost_system STATIC IMPORTED)
#set_target_properties(libboost_system PROPERTIES
#  IMPORTED_LOCATION ${BOOST_SYSTEM_LIB}
#  IMPORTED_IMPLIB ${BOOST_SYSTEM_LIB}
#  INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
#  FOLDER 3rdParty
#)
#if (boost_needs_build)
#  add_dependencies(libboost_system boost_build)
#endif()
#
#add_library(libboost_thread STATIC IMPORTED)
#set_target_properties(libboost_thread PROPERTIES
#  IMPORTED_LOCATION ${BOOST_THREAD_LIB}
#  IMPORTED_IMPLIB ${BOOST_THREAD_LIB}
#  INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
#  FOLDER 3rdParty
#)
#if (boost_needs_build)
#  add_dependencies(libboost_thread boost_build)
#endif()
