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
if(NOT DEFINED git)
  message(FATAL_ERROR "git not defined")
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

# Reads a file with the trailing whitespace of every line removed, and CR with it,
# which is the difference git is told to ignore below.
function (readnormalized path out)
  if (NOT EXISTS "${path}")
    # Not a comparison this can answer - say so in a way that cannot equal the
    # other side, and let the diff below name the file.
    set(${out} "<no ${path}>" PARENT_SCOPE)
    return()
  endif()
  file(READ "${path}" contents)
  string(REGEX REPLACE "[ \t\r]+\n" "\n" contents "${contents}")
  # A final newline is not content: git counts its absence as whitespace at the
  # end of the last line and ignores it too.
  string(REGEX REPLACE "\n$" "" contents "${contents}")
  string(REGEX REPLACE "[ \t\r]+$" "" contents "${contents}")
  set(${out} "${contents}" PARENT_SCOPE)
endfunction()

function (compareresult scriptname result optimized)
  set (outname "${scriptname}.out")
  if (optimized AND EXISTS "${scriptname}.optimized.out")
    set (outname "${scriptname}.optimized.out")
  endif()

  # Compared here rather than by a git: this runs once per script per pass, and
  # starting a process to compare two small files costs more than compiling and
  # running the script did. git still draws the mismatch, since that output is what
  # a failure is read from - but only on the path that has one.
  readnormalized("${outname}" expected)
  readnormalized("${scriptname}.tst" actual)
  if (expected STREQUAL actual)
    set(${result} 1 PARENT_SCOPE)
    return()
  endif()

  execute_process(
    COMMAND ${git} diff --no-index --ignore-space-at-eol ${outname} "${scriptname}.tst"
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(${result} 0 PARENT_SCOPE)
  message(SEND_ERROR "${scriptname}.src failed")
  cleanup(${scriptname})
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
function (testwithcompiler)
  set (options FORMAT SHORTCIRCUIT)
  set (value )
  set (multivalue )
  cmake_parse_arguments(test "${options}" "${value}" "${multivalue}" ${ARGN})
  set (ecompilearg )
  set (testname "")
  if(test_SHORTCIRCUIT)
    set (ecompilearg -S)
    set (testname " [shortcircuit]")
  elseif(test_FORMAT)
    set (testname " [formatted]")
  endif()
  # Only add -Z for ast tests
  if(${subtest} MATCHES "ast")
    # Currently don't have formatting for typing nodes, so ignore that subtest
    # when testing formatting.
    if (test_FORMAT)
      return()
    endif()
    list (APPEND ecompilearg -Z)
  endif()
  
  file(GLOB scripts RELATIVE ${testdir} ${testdir}/${subtest}/*)
  foreach(script ${scripts})
    string(FIND "${script}" ".src" out)
    if(${out} LESS 0)
      continue()
    endif()
    string(REPLACE ".src" "" scriptname "${script}")

    message("${script}${testname}")
    if (test_FORMAT)
      if (EXISTS "${testdir}/${scriptname}.out")
        string(REPLACE ".src" ".unformatted.src" unformattedscript "${script}")
        configure_file(${testdir}/${script} ${testdir}/${unformattedscript} COPYONLY)
        execute_process( COMMAND ${ecompile} -Fi -q -C ecompile.cfg ${script}
          RESULT_VARIABLE ecompile_format_res
          OUTPUT_VARIABLE ecompile_format_out
          ERROR_VARIABLE ecompile_format_out)
      else()
        continue()
      endif()
    endif()

    if (EXISTS "${testdir}/${scriptname}.out" OR EXISTS "${testdir}/${scriptname}.err")
      execute_process( COMMAND ${ecompile} ${ecompilearg} -l -C ecompile.cfg ${script}
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
          if(test_FORMAT AND DEFINED keepfailedformats)
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
            compareresult(${scriptname} success ${test_SHORTCIRCUIT})
            # no message needed compare prints
          endif()
        endif()
      endif()
      cleanup(${scriptname})
    endif()
  endforeach()
endfunction()

testwithcompiler()
testwithcompiler(SHORTCIRCUIT)
testwithcompiler(FORMAT)
