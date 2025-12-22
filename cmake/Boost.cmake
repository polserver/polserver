message("* libboost")
set (BOOST_SOURCE_DIR "${EXT_DOWNLOAD_DIR}/boost_1_89_0")
set (BOOST_STAGE_LIB_DIR "${BOOST_SOURCE_DIR}/stage/lib")

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

#if (NOT DEFINED BOOST_TOOLSET)
#  message(FATAL_ERROR "Unknown boost toolset to build")
#endif()
set (BOOST_ARC "")
if (${windows})
  set (BOOST_CONFIGURE_COMMAND "bootstrap.bat")
  set (BOOST_BUILD_COMMAND "b2.exe")
  set (BOOST_STACKTRACE_LIB2 "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_windbg.lib")
  set (BOOST_STACKTRACE_LIB "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_from_exception.lib")
  if (msvc)
    set (BOOST_CXX_FLAGS "/MT -DBOOST_STACKTRACE_LINK")
  else()
    set (BOOST_CXX_FLAGS "-fms-runtime-lib=static -DBOOST_STACKTRACE_LINK")
  endif()
else()
  set (BOOST_CONFIGURE_COMMAND "./bootstrap.sh")
  set (BOOST_BUILD_COMMAND "./b2")
  set (BOOST_STACKTRACE_LIB2 "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_backtrace.a")
  set (BOOST_STACKTRACE_LIB "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_from_exception.a" )

  set (BOOST_CXX_FLAGS "-std=c++${CMAKE_CXX_STANDARD} -DBOOST_STACKTRACE_LINK")
  if (clang)
    set (BOOST_STACKTRACE_LIB2 "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_addr2line.a")
  endif()
  if (APPLE)
    foreach(OSX_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
      set (BOOST_CXX_FLAGS "${BOOST_CXX_FLAGS} -arch ${OSX_ARCHITECTURE}")
    endforeach()
    set (BOOST_ARC "architecture=arm+x86")
    set (BOOST_STACKTRACE_LIB "${BOOST_STAGE_LIB_DIR}/libboost_stacktrace_basic.a")
    set (BOOST_STACKTRACE_LIB2 "")
  else()
    set (BOOST_CXX_FLAGS "${BOOST_CXX_FLAGS} -fPIC")
  endif()
endif()

set(boost_needs_extract FALSE)
set(boost_needs_build FALSE)
if (NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
  message("  - will extract")
  ExternalProject_Add(libboost_ext
    URL "https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-b2-nodocs.tar.gz"
    SOURCE_DIR "${BOOST_SOURCE_DIR}"
    CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND} --with-toolset=${BOOST_TOOLSET}
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    URL_HASH SHA256=aa25e7b9c227c21abb8a681efd4fe6e54823815ffc12394c9339de998eb503fb
    BUILD_BYPRODUCTS "${BOOST_SOURCE_DIR}"
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    BUILD_IN_SOURCE 1
    LOG_OUTPUT_ON_FAILURE 1
    DOWNLOAD_EXTRACT_TIMESTAMP 1
    EXCLUDE_FROM_ALL 1
  )
  set_target_properties (libboost_ext PROPERTIES FOLDER 3rdParty)
  set(boost_needs_extract TRUE)
else()
  message("  - will not extract")
endif()

if(NOT EXISTS ${BOOST_STACKTRACE_LIB})
  message("  - will build in ${BOOST_SOURCE_DIR} toolset=${BOOST_TOOLSET} cxxflags=${BOOST_CXX_FLAGS}")
  ExternalProject_Add(boost_build
          SOURCE_DIR ${BOOST_SOURCE_DIR}
          INSTALL_COMMAND ""
          CONFIGURE_COMMAND ""
          DOWNLOAD_COMMAND ""
          BUILD_COMMAND ${BOOST_BUILD_COMMAND} cxxflags=${BOOST_CXX_FLAGS} cflags=${BOOST_CXX_FLAGS} address-model=${ARCH_BITS} toolset=${BOOST_TOOLSET} ${BOOST_ARC} variant=release link=static runtime-link=static --layout=system --with-stacktrace stage
          BUILD_BYPRODUCTS ${BOOST_STACKTRACE_LIB} ${BOOST_STACKTRACE_LIB2}
          LOG_BUILD 1
          BUILD_IN_SOURCE 1
          LOG_OUTPUT_ON_FAILURE 1
          )
  if (boost_needs_extract)
    add_dependencies(boost_build libboost_ext)
  endif()
  set_target_properties (boost_build PROPERTIES FOLDER 3rdParty)
  set(boost_needs_build TRUE)
else()
  message("  - will not build")
endif()

# imported target to add include/lib dir and additional dependencies
add_library(libboost_headers INTERFACE IMPORTED)
set_target_properties(libboost_headers PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
)
if (boost_needs_extract)
  add_dependencies(libboost_headers libboost_ext)
endif()

if (USE_BOOST_HEADER_STACKTRACE)
  add_library(libboost_stacktrace INTERFACE IMPORTED)
  set_target_properties(libboost_stacktrace PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
    INTERFACE_COMPILE_DEFINITIONS BOOST_STACKTRACE_USE_ADDR2LINE
    FOLDER 3rdParty
  )
  set(BOOST_STACKTRACE_LIB2)
else()
  set_target_properties(libboost_stacktrace PROPERTIES
    IMPORTED_LOCATION ${BOOST_STACKTRACE_LIB}
    IMPORTED_IMPLIB ${BOOST_STACKTRACE_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${BOOST_SOURCE_DIR}
    INTERFACE_COMPILE_DEFINITIONS BOOST_STACKTRACE_LINK
    FOLDER 3rdParty
  )
endif()
if (${windows})
  set_property(TARGET libboost_stacktrace APPEND
      PROPERTY INTERFACE_LINK_LIBRARIES
        dbgeng
        ole32
        ${BOOST_STACKTRACE_LIB2}
    )
else()
  set_property(TARGET libboost_stacktrace APPEND
      PROPERTY INTERFACE_LINK_LIBRARIES
        ${BOOST_STACKTRACE_LIB2}
        dl
        $<$<NOT:$<PLATFORM_ID:Darwin>>:backtrace>
    )
endif()
if (boost_needs_build)
  add_dependencies(libboost_stacktrace boost_build)
endif()
