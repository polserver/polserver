set(testsuite ${CMAKE_CURRENT_SOURCE_DIR}/testsuite/pol)

# cleanup as initial fixture, to have at the end of the run still all testdata
if (${CMAKE_VERSION} VERSION_LESS "3.17")
  add_test(NAME cleantestdir
    COMMAND ${CMAKE_COMMAND} -E remove_directory coretest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
else()
  add_test(NAME cleantestdir
    COMMAND ${CMAKE_COMMAND} -E rm -rf coretest
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif()
set_tests_properties(cleantestdir PROPERTIES FIXTURES_SETUP client)

# generate client files, minimal distro and needed core cfgs
add_test(NAME testenv
  COMMAND poltool testenv
    outdir=coretest
    width=192
    height=192
    hsa=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(testenv PROPERTIES DEPENDS cleantestdir)
set_tests_properties(testenv PROPERTIES FIXTURES_SETUP client)

add_test(NAME shard_cfgfiles
  COMMAND ${CMAKE_COMMAND} -E copy ${testsuite}/pol.cfg ${testsuite}/uoconvert.cfg coretest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_cfgfiles PROPERTIES DEPENDS testenv)
set_tests_properties(shard_cfgfiles PROPERTIES FIXTURES_SETUP shard)

add_test(NAME shard_ecompile_cfg
  COMMAND ${CMAKE_COMMAND}
    -Dtestdir=${CMAKE_BINARY_DIR}/coretest
    -Decompile_cfg=${testsuite}/ecompile.cfg
    -Dmodules=${CMAKE_SOURCE_DIR}/pol-core/support/scripts
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/core_tests_ecompile_cfg.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_ecompile_cfg PROPERTIES DEPENDS testenv)
set_tests_properties(shard_ecompile_cfg PROPERTIES FIXTURES_SETUP shard)

add_test(NAME shard_testscript
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${testsuite}/scripts coretest/scripts
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_testscript PROPERTIES DEPENDS testenv)
set_tests_properties(shard_testscript PROPERTIES FIXTURES_SETUP shard)

add_test(NAME shard_testconfig
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${testsuite}/config coretest/config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_testconfig PROPERTIES DEPENDS testenv)
set_tests_properties(shard_testconfig PROPERTIES FIXTURES_SETUP shard)

add_test(NAME shard_testpkgs
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${testsuite}/testpkgs coretest/testpkgs
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_testpkgs PROPERTIES DEPENDS testenv)
set_tests_properties(shard_testpkgs PROPERTIES FIXTURES_SETUP shard)

# uoconvert part

add_test(NAME uoconvert_map
  COMMAND uoconvert map britannia 
    width=192 height=192 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_map PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_map PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_statics
  COMMAND uoconvert statics britannia 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_statics PROPERTIES DEPENDS uoconvert_map)
set_tests_properties( uoconvert_statics PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_statics PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_maptile
  COMMAND uoconvert maptile britannia 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_maptile PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_maptile PROPERTIES DEPENDS uoconvert_map)
set_tests_properties( uoconvert_maptile PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_tiles
  COMMAND uoconvert tiles
    uodata=client
    maxtileid=0x3fff
    outdir=config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_tiles PROPERTIES FIXTURES_REQUIRED "client;shard")
set_tests_properties( uoconvert_tiles PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_landtiles
  COMMAND uoconvert landtiles
    uodata=client
    maxtileid=0x3fff
    outdir=config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_landtiles PROPERTIES FIXTURES_REQUIRED "client;shard")
set_tests_properties( uoconvert_landtiles PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_multis
  COMMAND uoconvert multis
    uodata=client
    maxtileid=0x3fff
    outdir=config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_multis PROPERTIES FIXTURES_REQUIRED "client;shard")
set_tests_properties( uoconvert_multis PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME shard_ecompile
  COMMAND ecompile -A -x
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( shard_ecompile PROPERTIES FIXTURES_REQUIRED shard)
set_tests_properties( shard_ecompile PROPERTIES FIXTURES_SETUP ecompile)

# first test run
add_test(NAME shard_test_1
  COMMAND pol
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( shard_test_1 PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")
# needed for test_env
set_tests_properties( shard_test_1 PROPERTIES ENVIRONMENT "POLCORE_TEST=1;POLCORE_TEST_RUN=1;POLCORE_TEST_NOACCESS=foo")
set_tests_properties(shard_test_1 PROPERTIES FIXTURES_SETUP shard_test)

# second test run
add_test(NAME shard_test_2
  COMMAND pol
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( shard_test_2 PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")
set_tests_properties( shard_test_2 PROPERTIES ENVIRONMENT "POLCORE_TEST_RUN=2")
set_tests_properties( shard_test_2 PROPERTIES FIXTURES_REQUIRED shard_test)
