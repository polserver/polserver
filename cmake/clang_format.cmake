#clang-format all modified/new cpp/h/hpp files in given directory (pol-core)
if(NOT DEFINED GIT)
  message(FATAL_ERROR "git not defined")
endif()
if(NOT DEFINED CLANG_FORMAT)
  message(FATAL_ERROR "clang-format not defined")
endif()

execute_process( COMMAND ${GIT} ls-files -m -o --exclude-standard
  WORKING_DIRECTORY ${WORKDIR}
  RESULT_VARIABLE git_res
  OUTPUT_VARIABLE git_out
  ERROR_VARIABLE git_out
)
STRING(REGEX REPLACE "\n" ";" git_out "${git_out}")

foreach(file ${git_out})
  get_filename_component(ext ${file} EXT)
  string(TOLOWER ${ext} ext)
  if(${ext} STREQUAL ".cpp" OR ${ext} STREQUAL ".h" OR ${ext} STREQUAL ".hpp")
    message(${file})
    execute_process(COMMAND ${CLANG_FORMAT} -i -style=file ${file}
      WORKING_DIRECTORY ${WORKDIR}
    )
  endif()
endforeach()
