find_package(Python3 COMPONENTS Interpreter REQUIRED QUIET)
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${testdir}/ecompile_watch_test.py ${ecompile}
  COMMAND_ECHO STDOUT
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  RESULT_VARIABLE res
  TIMEOUT 600
)
if(NOT "${res}" STREQUAL "0")
  message(SEND_ERROR "${res}")
endif()
