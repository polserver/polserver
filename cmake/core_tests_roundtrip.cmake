# World-save round trip.
#
# Copies the world the previous pass wrote aside, boots the shard once more so it loads and
# re-saves that world without touching it (POLCORE_TEST_RUN=3 -> scripts/roundtrip.src), then
# compares the two saves through testsuite/worlddiff/worlddiff.py.
#
# Run with the working directory set to the coretest shard, so the paths below are relative
# to it. Each execute_process takes exactly one COMMAND: several COMMANDs in one call is a
# pipeline, not a sequence.

find_package(Python3 COMPONENTS Interpreter REQUIRED QUIET)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf data_before
  RESULT_VARIABLE res
)
if(NOT "${res}" STREQUAL "0")
  message(FATAL_ERROR "could not clear data_before: ${res}")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_directory data data_before
  RESULT_VARIABLE res
)
if(NOT "${res}" STREQUAL "0")
  message(FATAL_ERROR "could not copy the world aside: ${res}")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E env POLCORE_TEST_RUN=3 ${pol}
  COMMAND_ECHO STDOUT
  RESULT_VARIABLE res
  TIMEOUT 300
)
if(NOT "${res}" STREQUAL "0")
  message(FATAL_ERROR "the round-trip shard run failed: ${res}")
endif()

execute_process(
  COMMAND ${Python3_EXECUTABLE} ${testdir}/worlddiff/worlddiff.py data_before data --verbose
  COMMAND_ECHO STDOUT
  RESULT_VARIABLE res
)
if(NOT "${res}" STREQUAL "0")
  message(SEND_ERROR "${res}")
endif()
