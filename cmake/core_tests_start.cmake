find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED QUIET)
# The budget below covers the whole pipeline, and the shard test runs some ten minutes
# on a loaded machine, so it has to be several times that rather than close to it.
# Overrunning it comes back as the res below and is reported as a shard failure, which
# is the most misleading way this can fail.
set(SHARD_TEST_TIMEOUT 1800)
if(testemail)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${testdir}/testclient/pyuo/testclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/smtpd/smtpd.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/deafclient/deafclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/rawpeer/rawpeer.py
    COMMAND ${pol}
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE res
    TIMEOUT ${SHARD_TEST_TIMEOUT}
  )
else()
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${testdir}/testclient/pyuo/testclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/deafclient/deafclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/rawpeer/rawpeer.py
    COMMAND ${pol}
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE res
    TIMEOUT ${SHARD_TEST_TIMEOUT}
  )
endif()
if(NOT "${res}" STREQUAL "0")
  message(SEND_ERROR "${res}")
endif()
