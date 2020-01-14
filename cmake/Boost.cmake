message("* boost")
set (BOOST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/boost_1_67_0")
set (BOOST_STAGE_LIB_DIR "${BOOST_SOURCE_DIR}/stage/lib")
if (${windows})
  set (BOOST_CONFIGURE_COMMAND "bootstrap.bat")
  set (BOOST_BUILD_COMMAND "b2.exe")
  set (BOOST_FILESYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_filesystem-vc141-mt-s-${ARCH_STRING}-1_67.lib" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system-vc141-mt-s-${ARCH_STRING}-1_67.lib" )
else()  
  set (BOOST_CONFIGURE_COMMAND "./bootstrap.sh")
  set (BOOST_BUILD_COMMAND "./b2")
  set (BOOST_FILESYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_filesystem.a" )
  set (BOOST_SYSTEM_LIB "${BOOST_STAGE_LIB_DIR}/libboost_system.a" )
endif()

if(NOT EXISTS ${BOOST_FILESYSTEM_LIB} OR NOT EXISTS ${BOOST_SYSTEM_LIB})
  ExternalProject_Add(boost
    URL "${BOOST_SOURCE_DIR}/../boost_1_67_0.tar.bz2"
    SOURCE_DIR "${BOOST_SOURCE_DIR}" 
    CONFIGURE_COMMAND ${BOOST_CONFIGURE_COMMAND}
    INSTALL_COMMAND ""
    # BUILD_COMMAND ""
    BUILD_COMMAND ${BOOST_BUILD_COMMAND} link=static variant=release runtime-link=static address-model=${ARCH_BITS} --with-filesystem stage
    BUILD_BYPRODUCTS ${BOOST_FILESYSTEM_LIB} ${BOOST_SYSTEM_LIB}
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    BUILD_IN_SOURCE 1
    LOG_OUTPUT_ON_FAILURE 1
  )
  set_target_properties (boost PROPERTIES FOLDER 3rdParty)
else()
  message("Boost already extracted")
endif()

