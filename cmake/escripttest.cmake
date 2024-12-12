#EScript test suite

if(NOT DEFINED testdir)
  message(FATAL_ERROR "testdir not defined")
endif()
if(NOT DEFINED subtest)
  message(FATAL_ERROR "subtest not defined")
endif()
if(NOT DEFINED runecl)
  message(FATAL_ERROR "runecl not defined")
endif()
if(NOT DEFINED ecompile)
  message(FATAL_ERROR "ecompile not defined")
endif()

# Uncomment to not delete failed formatted files (will be saved as {scriptname}.formatted.src)
# set(keepfailedformats TRUE)

function (cleanup scriptname)
  foreach (ext .ecl .tst .lst .ast .unformatted.src .formatted.src .dbg)
    if(EXISTS "${scriptname}${ext}")
      if ("${ext}" STREQUAL ".formatted.src" AND DEFINED keepfailedformats)
        continue()
      endif()
      if("${ext}" STREQUAL ".unformatted.src")
        configure_file(${scriptname}${ext} ${scriptname}.src COPYONLY)
      endif()
      file(REMOVE "${scriptname}${ext}")
    endif()
  endforeach()
endfunction()

function (readfile file content content_len)
  # read given file into string list
  FILE(READ ${file} contents)
  #keep original ;
  STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
  #remove last newline
  STRING(REGEX REPLACE "\n$" "" contents "${contents}")
  #cmake does not really support entries add _
  STRING(REGEX REPLACE "\n\n" "\n_\n" contents "${contents}")
  #each newline a new list element
  STRING(REGEX REPLACE "\n" ";" contents "${contents}")
  list(LENGTH contents len)
  set(${content} ${contents} PARENT_SCOPE)
  set(${content_len} ${len} PARENT_SCOPE)
endfunction()

function (compareresult scriptname result)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files "${scriptname}.out" "${scriptname}.tst"
    RESULT_VARIABLE test_not_successful
    OUTPUT_QUIET
    ERROR_QUIET
  )
  if (NOT ${test_not_successful})
    set(${result} 1 PARENT_SCOPE)
    return()
  endif()
  set(${result} 0 PARENT_SCOPE)
  readfile("${scriptname}.out" outcontent outlen)
  readfile("${scriptname}.tst" tstcontent tstlen)
  if (${outlen} EQUAL ${tstlen})
    math(EXPR looprange ${outlen}-1)
    foreach(i RANGE ${looprange})
      list(GET outcontent ${i} out)
      list(GET tstcontent ${i} tst)
      if (NOT ${out} STREQUAL ${tst})
        math(EXPR line ${i}+1)
        message("${scriptname}.src failed:")
        message("Line ${line} Expected:\n${out}\nGot:\n${tst}")
        cleanup(${scriptname})
        set(${result} 0 PARENT_SCOPE)
        message(SEND_ERROR "Line differs")
      endif()
    endforeach()
  else()
    message("\"${tstcontent}\"")
    message(SEND_ERROR "Testdata length differs")
    cleanup(${scriptname})
  endif()
endfunction()

function (checkprocfailure output errorfile result)
  # correct error msg from process?
  file(READ "${errorfile}" contents)
  string(FIND "${output}" "${contents}" err_found)
  if(${err_found} LESS 0)
    message(SEND_ERROR "Expected:\n${contents}\nGot:${output}")
    set(${result} 0 PARENT_SCOPE)
  else()
    set(${result} 1 PARENT_SCOPE)
  endif()
endfunction()

# start of test
function (testwithcompiler formatoption)
  # Only add -Z for ast tests
  if(${subtest} MATCHES "ast")
    # Currently don't have formatting for typing nodes, so ignore that subtest
    # when testing formatting.
    if (NOT ${formatoption} STREQUAL "")
      return()
    endif()
    set(astoption "-Z")
  else()
    set(astoption "")
  endif()

  file(GLOB scripts RELATIVE ${testdir} ${testdir}/${subtest}/*)
  foreach(script ${scripts})
    string(FIND "${script}" ".src" out)
    if(${out} LESS 0)
      continue()
    endif()
    string(REPLACE ".src" "" scriptname "${script}")

    if (EXISTS "${testdir}/${scriptname}.out" AND NOT ${formatoption} STREQUAL "")
      message("${script} [formatted]")
      string(REPLACE ".src" ".unformatted.src" unformattedscript "${script}")
      configure_file(${testdir}/${script} ${testdir}/${unformattedscript} COPYONLY)
      execute_process( COMMAND ${ecompile} ${formatoption} ${astoption} -q -C ecompile.cfg ${script}
        RESULT_VARIABLE ecompile_format_res
        OUTPUT_VARIABLE ecompile_format_out
        ERROR_VARIABLE ecompile_format_out)
    else()
      message(${script})
    endif()

    if (EXISTS "${testdir}/${scriptname}.out" OR EXISTS "${testdir}/${scriptname}.err")
      execute_process( COMMAND ${ecompile} ${astoption} -l -q -C ecompile.cfg ${script}
        RESULT_VARIABLE ecompile_res
        OUTPUT_VARIABLE ecompile_out
        ERROR_VARIABLE ecompile_out)
      if (EXISTS "${testdir}/${scriptname}.err")
        if("${ecompile_res}" STREQUAL "0")
          cleanup(${scriptname})
          message(SEND_ERROR "${scriptname}.src compiled, but should not")
        else()
          checkprocfailure("${ecompile_out}" "${testdir}/${scriptname}.err" success)
          if (NOT ${success})
            cleanup(${scriptname})
            message(SEND_ERROR "${scriptname}.src failed with different error")
          endif()
        endif()
      else()
        if(NOT "${ecompile_res}" STREQUAL "0")
          if(NOT ${formatoption} STREQUAL "" AND DEFINED keepfailedformats)
            configure_file(${testdir}/${script} ${testdir}/${scriptname}.formatted.src)
          endif()
          cleanup(${scriptname})
          message(SEND_ERROR "${scriptname}.src did not compile")
          message(${ecompile_out})
        endif()
        execute_process( COMMAND ${runecl} -q "${scriptname}.ecl"
          OUTPUT_FILE "${scriptname}.tst"
          RESULT_VARIABLE runecl_res
          ERROR_VARIABLE runecl_out)
        if (EXISTS "${testdir}/${scriptname}.exr")
          if(NOT "${runecl_res}" STREQUAL "0")
            checkprocfailure("${runecl_out}" "${testdir}/${scriptname}.exr" success)
            if (NOT ${success})
              cleanup(${scriptname})
              message(SEND_ERROR "${scriptname}.ecl failed with different error")
            endif()
          else()
            cleanup(${scriptname})
            message(SEND_ERROR "${scriptname}.ecl should have failed")
          endif()
        else()
          if(NOT "${runecl_res}" STREQUAL "0")
            file(READ "${scriptname}.tst" contents)
            message("${runecl_out} ${contents}")
            cleanup(${scriptname})
            message(SEND_ERROR "${scriptname}.ecl did not run")
          else()
            compareresult(${scriptname} success)
            # no message needed compare prints
          endif()
        endif()
      endif()
      cleanup(${scriptname})
    endif()
  endforeach()
endfunction()

testwithcompiler("")
testwithcompiler("-Fi")
