# The interpreter configure found and checked for aiosmtpd. A find_package in script mode can
# find a different one, which then lacks the module.
set(Python3_EXECUTABLE "${python}")
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
    TIMEOUT 900
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
    TIMEOUT 900
  )
endif()
if(NOT "${res}" STREQUAL "0")
  message(SEND_ERROR "${res}")
endif()
