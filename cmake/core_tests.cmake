set(testsuite ${CMAKE_CURRENT_SOURCE_DIR}/testsuite/pol)

# cleanup as initial fixture, to have at the end of the run still all testdata
add_test(NAME cleantestdir
  COMMAND ${CMAKE_COMMAND} -E rm -rf coretest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(cleantestdir PROPERTIES FIXTURES_SETUP "client;initial_cleanup")

# uoconvert tests

add_test(NAME uoconvert_test_map0_hsa
  COMMAND poltool testfiles
    outdir=coretest/convert_test/client
    width=768
    height=4096
    mapid=0
    hsa=1
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(uoconvert_test_map0_hsa PROPERTIES FIXTURES_REQUIRED initial_cleanup)
set_tests_properties(uoconvert_test_map0_hsa PROPERTIES FIXTURES_SETUP uoconvert_tests_map0)

add_test(NAME uoconvert_test_setup_map1_no_hsa
  COMMAND poltool testfiles
    outdir=coretest/convert_test/client
    width=640
    height=4096
    mapid=1
    hsa=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(uoconvert_test_setup_map1_no_hsa PROPERTIES FIXTURES_REQUIRED initial_cleanup)
set_tests_properties(uoconvert_test_setup_map1_no_hsa PROPERTIES FIXTURES_SETUP uoconvert_tests_map1)

add_test(NAME uoconvert_test_convert_map0
  COMMAND uoconvert map realm=britannia
    mapid=0
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest/convert_test
)
set_tests_properties( uoconvert_test_convert_map0 PROPERTIES FIXTURES_REQUIRED uoconvert_tests_map0)

add_test(NAME uoconvert_test_convert_map1
  COMMAND uoconvert map realm=britannia2
    mapid=1
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest/convert_test
)
set_tests_properties( uoconvert_test_convert_map1 PROPERTIES FIXTURES_REQUIRED uoconvert_tests_map1)



# generate client files, minimal distro and needed core cfgs
add_test(NAME testenv_map1
  COMMAND poltool testfiles
    outdir=coretest/client
    width=1600
    height=1600
    mapid=1
    hsa=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(testenv_map1 PROPERTIES DEPENDS cleantestdir)
set_tests_properties(testenv_map1 PROPERTIES FIXTURES_SETUP client)

add_test(NAME testenv
  COMMAND poltool testenv
    outdir=coretest
    width=192
    height=192
    mapid=0
    hsa=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(testenv PROPERTIES DEPENDS testenv_map1)
set_tests_properties(testenv PROPERTIES FIXTURES_SETUP client)

add_test(NAME shard_files
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${testsuite} coretest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_files PROPERTIES DEPENDS testenv)
set_tests_properties(shard_files PROPERTIES FIXTURES_SETUP shard)

add_test(NAME shard_ecompile_cfg
  COMMAND ${CMAKE_COMMAND}
    -Dtestdir=${CMAKE_BINARY_DIR}/coretest
    -Decompile_cfg=${testsuite}/ecompile.cfg
    -Dmodules=${CMAKE_SOURCE_DIR}/pol-core/support/scripts
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/core_tests_ecompile_cfg.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(shard_ecompile_cfg PROPERTIES DEPENDS shard_files)
set_tests_properties(shard_ecompile_cfg PROPERTIES FIXTURES_SETUP shard)


# uoconvert part

add_test(NAME uoconvert_map
  COMMAND uoconvert map realm=britannia
    width=192 height=192
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_map PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_map PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_statics
  COMMAND uoconvert statics realm=britannia
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_statics PROPERTIES DEPENDS uoconvert_map)
set_tests_properties( uoconvert_statics PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_statics PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_maptile
  COMMAND uoconvert maptile realm=britannia
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_maptile PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_maptile PROPERTIES DEPENDS uoconvert_map)
set_tests_properties( uoconvert_maptile PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_map2
  COMMAND uoconvert map realm=britannia2
    width=1600
    height=1600
    mapid=1
    uodata=client
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_map2 PROPERTIES DEPENDS uoconvert_maptile)
set_tests_properties( uoconvert_map2 PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_map2 PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_statics2
  COMMAND uoconvert statics realm=britannia2
    uodata=client
    mapid=1
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_statics2 PROPERTIES DEPENDS uoconvert_map2)
set_tests_properties( uoconvert_statics2 PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_statics2 PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_maptile2
  COMMAND uoconvert maptile realm=britannia2
    uodata=client
    mapid=1
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_maptile2 PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_maptile2 PROPERTIES DEPENDS uoconvert_map2)
set_tests_properties( uoconvert_maptile2 PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_tiles
  COMMAND uoconvert tiles
    uodata=client
    outdir=config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_tiles PROPERTIES FIXTURES_REQUIRED "client;shard")
set_tests_properties( uoconvert_tiles PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_landtiles
  COMMAND uoconvert landtiles
    uodata=client
    outdir=config
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_landtiles PROPERTIES FIXTURES_REQUIRED "client;shard")
set_tests_properties( uoconvert_landtiles PROPERTIES FIXTURES_SETUP uoconvert)

add_test(NAME uoconvert_multis
  COMMAND uoconvert multis
    uodata=client
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
find_package(Python3 QUIET COMPONENTS Interpreter)
if (${Python3_FOUND})
  execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import aiosmtpd"
      RESULT_VARIABLE _aiosmtpd_status
      OUTPUT_QUIET
      ERROR_QUIET
  )
  if(_aiosmtpd_status EQUAL 0)
    set(HAS_AIOSMTPD TRUE)
  else()
    set(HAS_AIOSMTPD FALSE)
  endif()

  add_test(NAME shard_test_1
    COMMAND ${CMAKE_COMMAND}
      -Dpol=$<TARGET_FILE:pol>
      -Dtestdir=${CMAKE_CURRENT_SOURCE_DIR}/testsuite
      -Dtestemail=${HAS_AIOSMTPD}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/core_tests_start.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
  )
  add_test(NAME ecompile_watch_test
    COMMAND ${CMAKE_COMMAND}
      -Decompile=$<TARGET_FILE:ecompile>
      -Dtestdir=${CMAKE_CURRENT_SOURCE_DIR}/testsuite
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/core_tests_ecompile_watch.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
    )
    set_tests_properties(ecompile_watch_test PROPERTIES FIXTURES_REQUIRED shard)
else()
  message(" - core test without testclient python3 not found")
  add_test(NAME shard_test_1
    COMMAND pol
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
  )
endif()
set_tests_properties( shard_test_1 PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")
# needed for test_env
set_tests_properties( shard_test_1 PROPERTIES ENVIRONMENT "POLCORE_TEST=1;POLCORE_TEST_RUN=1;POLCORE_TEST_NOACCESS=foo;POLCORE_TESTCLIENT=${Python3_FOUND};POLCORE_TESTEMAIL=${HAS_AIOSMTPD}")
set_tests_properties(shard_test_1 PROPERTIES FIXTURES_SETUP shard_test)

# second test run
add_test(NAME shard_test_2
  COMMAND pol
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( shard_test_2 PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")
set_tests_properties( shard_test_2 PROPERTIES ENVIRONMENT "POLCORE_TEST_RUN=2")
set_tests_properties( shard_test_2 PROPERTIES FIXTURES_REQUIRED shard_test)

# unit test
add_test(NAME unittest_pol
  COMMAND pol -test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( unittest_pol PROPERTIES FIXTURES_REQUIRED "client;shard;uoconvert;ecompile")
