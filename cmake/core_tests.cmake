
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
add_test(NAME testdir
  COMMAND ${CMAKE_COMMAND} -E make_directory coretest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(testdir PROPERTIES DEPENDS cleantestdir)

set_tests_properties(testdir PROPERTIES FIXTURES_SETUP client)
add_test(NAME testfiles
  COMMAND ../../bin/poltool testfiles
    outdir=client
    width=192
    height=192
    hsa=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties(testfiles PROPERTIES DEPENDS testdir)
set_tests_properties(testfiles PROPERTIES FIXTURES_SETUP client)

# start of real tests

add_test(NAME uoconvert_map
  COMMAND ../../bin/uoconvert map britannia 
    width=192 height=192 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_map PROPERTIES FIXTURES_REQUIRED client)

add_test(NAME uoconvert_statics
  COMMAND ../../bin/uoconvert statics britannia 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_statics PROPERTIES DEPENDS uoconvert_map)
set_tests_properties( uoconvert_statics PROPERTIES FIXTURES_REQUIRED client)

add_test(NAME uoconvert_maptile
  COMMAND ../../bin/uoconvert maptile britannia 
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_maptile PROPERTIES FIXTURES_REQUIRED client)
set_tests_properties( uoconvert_maptile PROPERTIES DEPENDS uoconvert_map)

add_test(NAME uoconvert_tiles
  COMMAND ../../bin/uoconvert tiles
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_tiles PROPERTIES FIXTURES_REQUIRED client)

add_test(NAME uoconvert_landtiles
  COMMAND ../../bin/uoconvert landtiles
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_landtiles PROPERTIES FIXTURES_REQUIRED client)

add_test(NAME uoconvert_multis
  COMMAND ../../bin/uoconvert multis
    uodata=client
    maxtileid=0x3fff
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coretest
)
set_tests_properties( uoconvert_multis PROPERTIES FIXTURES_REQUIRED client)

