if(GIT)
  execute_process(COMMAND ${GIT}
    log -1 --pretty=format:%h
    RESULT_VARIABLE status
    OUTPUT_VARIABLE GIT_REVISION
    ERROR_QUIET
  )
  if (status)
    set(GIT_REVISION "\"Non-Git\"")
  else()
    set(GIT_REVISION "\"${GIT_REVISION}\"")
  endif()
else()
  set(GIT_REVISION "\"Unknown\"")
endif()
    #message(${GIT})
configure_file(
  ${CFG_FILE} 
  ${TMP_FILE}
)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${TMP_FILE}
  ${OUT_FILE}
)
