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
  COMMAND ${CMAKE_COMMAND} -E copy ${testsuite}/pol.cfg ${testsuite}/ecompile.cfg ${testsuite}/uoconvert.cfg coretest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_cfgfiles PROPERTIES DEPENDS testenv)
set_tests_properties(shard_cfgfiles PROPERTIES FIXTURES_SETUP shard)

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

#TODO: on windows the binary path gets added to the cfg path entries
#add_test(NAME shard_ecompile_copy
#  COMMAND ${CMAKE_COMMAND} -E copy ecompile coretest
#  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#)
#set_tests_properties( shard_ecompile_copy PROPERTIES FIXTURES_REQUIRED shard)
#set_tests_properties( shard_ecompile_copy PROPERTIES FIXTURES_SETUP ecompile)

add_test(NAME shard_ecompile
  COMMAND ecompile -A
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
#set_tests_properties( shard_ecompile PROPERTIES DEPENDS shard_ecompile_copy)
set_tests_properties( shard_ecompile PROPERTIES FIXTURES_REQUIRED shard)
set_tests_properties( shard_ecompile PROPERTIES FIXTURES_SETUP ecompile)

add_test(NAME shard_test
  COMMAND pol
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( shard_test PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")

