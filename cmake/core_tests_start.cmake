find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED QUIET)
# POL runs through runpol.py, which tells the helpers when it has exited: a shard that stops
# before it listens on a port would otherwise keep them waiting out their deadlines.
if(testemail)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${testdir}/testclient/pyuo/testclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/smtpd/smtpd.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/deafclient/deafclient.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/rawpeer/rawpeer.py
    COMMAND ${Python3_EXECUTABLE} ${testdir}/runpol.py ${pol}
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
    COMMAND ${Python3_EXECUTABLE} ${testdir}/runpol.py ${pol}
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE res
    TIMEOUT 900
  )
endif()
if(NOT "${res}" STREQUAL "0")
  message(SEND_ERROR "${res}")
endif()
