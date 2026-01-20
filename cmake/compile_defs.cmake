function(set_compile_flags target is_executable)
  message("* ${target}")
  target_include_directories(${target}  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} #own folder
  )

  target_compile_definitions(${target} PRIVATE
    INC_PASSERT=1
    __STDC_CONSTANT_MACROS
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
    _REENTRANT
    ARCH_BITS=${ARCH_BITS}
  )
  if (${release})
    target_compile_definitions(${target} PRIVATE
      RELEASE_VERSION
    )
  endif()

  if (${debug})
    target_compile_definitions(${target} PRIVATE
      DEBUG_VERSION
    )
  endif()

  if (${linux})
    target_compile_definitions(${target} PRIVATE
      _GNU_SOURCE
      LINUX
    )
    if (${file_offset_bits})
      target_compile_definitions(${target} PRIVATE
        _FILE_OFFSET_BITS=64
      )
    endif()
  endif()

  if (${windows})
    target_compile_definitions(${target} PRIVATE
      _WIN32
      WIN32
      NOMINMAX
      WINDOWS
      WINVER=0x0601
      _WIN32_WINNT=0x0601
      _CONSOLE
    )
    if (${ARCH_BITS} EQUAL "64")
      target_compile_definitions(${target} PRIVATE
        _WIN64
      )
    endif()
  endif()
  
  target_compile_options(${target} PRIVATE
    $<${linux}:
      -fPIC
      -W
      -Wall
      -Wextra
    >
    
    $<$<AND:${FORCE_ARCH_BITS},${linux}>:
      -m${ARCH_BITS}
    >

    $<$<AND:${debug},${linux}>:
      -ggdb
    >

    $<${windows}:
      /MT # runtime lib
      /GS- # no buffer security
      /Gy # function level linking
      /Zi # pdb files
      /GF # string pooling
      /EHa # exception handling
      /W4
      /w45038
      /utf-8
    >
    $<$<AND:$<BOOL:${GCOV}>,${linux}>:
      -g
      -O0
      -fprofile-arcs
      -ftest-coverage
    >
  )

  if(${gcc} OR ${clang})
    if(${GCOV})
        target_link_libraries(${target} PRIVATE -fprofile-arcs)
    endif()
    if(${ENABLE_ASAN})
      target_compile_options(${target} PRIVATE -fsanitize=address)
      target_link_libraries(${target} PRIVATE  -fsanitize=address)
    endif()
    if(${ENABLE_USAN})
      target_compile_options(${target} PRIVATE -fsanitize=undefined)
      target_link_libraries(${target} PRIVATE  -fsanitize=undefined)
    endif()
    if(${ENABLE_MSAN})
      target_compile_options(${target} PRIVATE -fsanitize=memory)
      target_link_libraries(${target} PRIVATE  -fsanitize=memory)
    endif()
    if(${ENABLE_TSAN})
      target_compile_options(${target} PRIVATE -fsanitize=thread)
      target_link_libraries(${target} PRIVATE  -fsanitize=thread)
    endif()
    if (${ENABLE_ASAN} OR ${ENABLE_USAN} OR ${ENABLE_MSAN} OR ${ENABLE_TSAN})
      target_compile_options(${target} PRIVATE
        -g
        -fno-omit-frame-pointer
      )
    endif()
  endif()
  if (${windows})
    if (NOT "${CMAKE_GENERATOR}" MATCHES "Ninja")
      target_compile_options(${target} PRIVATE
        /MP # multiprocess build
      )
    endif()
  endif()

  if (${is_executable})
    if (${release} AND ${linux})
      if (${ENABLE_ASAN} OR ${ENABLE_USAN} OR ${ENABLE_MSAN} OR ${ENABLE_TSAN})
        #dont strip
      elseif(${STRIP_BINARIES})
        target_link_libraries(${target} PUBLIC
          -s # strip
        )
      endif()
    endif()
  endif()
  if (${windows})
    target_link_libraries(${target} PRIVATE
      -LARGEADDRESSAWARE # more then 2gb for 32bit
      -DEBUG # include debug information (otherwise attach to process does not work)
    )
    if (${release})
      target_link_libraries(${target} PRIVATE
        -OPT:REF # remove unused blocks
      )
    endif()
    if (${is_executable})
      # by default the absolute path during linking is stored in the executable
      # use just the targetname so that the pdbs get found when copied next to the executable
      # for eg CI purpose check clib ExceptionParser
      # this adds current binary dir also as search path (which is different for CI)
      set_target_properties(${target} PROPERTIES LINK_FLAGS "/PDBALTPATH:${target}.pdb")
    endif()
  endif()

  if(${linux})
    link_directories(
      /usr/local/lib
      /usr/lib
      /usr/lib${ARCH_BITS}
    )
  endif()

  set_target_properties(${target} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${output_bin_dir}
    LIBRARY_OUTPUT_DIRECTORY ${output_bin_dir}
    RUNTIME_OUTPUT_DIRECTORY ${output_bin_dir}
    PDB_NAME ${target}
    COMPILE_PDB_NAME ${target}
    PDB_OUTPUT_DIRECTORY ${output_bin_dir}
  )

  source_group_by_folder(${target})
endfunction()

function(source_group_by_folder target)
  set(last_dir "")
  set(files "")
  foreach(file ${${target}_sources})
    if(IS_ABSOLUTE ${file})
      file(RELATIVE_PATH relative_file "${CMAKE_CURRENT_SOURCE_DIR}" "${file}")
      get_filename_component(dir "${relative_path}" DIRECTORY)
    else()
      get_filename_component(dir "${file}" PATH)
    endif()
    if (NOT "${dir}" STREQUAL "${last_dir}")
      if (files)
        source_group("${last_dir}" FILES ${files})
      endif ()
      set(files "")
    endif (NOT "${dir}" STREQUAL "${last_dir}")
    set(files ${files} ${file})
    set(last_dir "${dir}")
  endforeach()
  if (files)
    source_group("${last_dir}" FILES ${files})
  endif ()
endfunction()

function (enable_pch target)
  # optional argument REUSE will use clib pch
  set(argn "${ARGN}")
  if (NOT NO_PCH)
    set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
    if ("${argn}" MATCHES "REUSE" AND REUSE_PCH)
      target_precompile_headers(${target} REUSE_FROM clib)
    else()
      if (NOT REUSE_PCH AND NOT EXISTS ${_pch_name})
        set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/../clib/StdAfx.h")
      endif()
      target_precompile_headers(${target} PRIVATE ${_pch_name})
    endif()
  endif()
endfunction()


function(use_benchmark target)
  if (ENABLE_BENCHMARK)
    target_link_libraries(${target} PUBLIC benchmark::benchmark)
  endif()
endfunction()

function(warning_suppression target)
  target_compile_options(${target} PRIVATE
    $<${windows}:
      /wd4996 #deprecated
      /wd4786 #identifier trunc for debug
    >
    $<${linux}:
    >
  )
endfunction()

function(use_tidy target)
  if (ENABLE_TIDY)
    set_target_properties(
      ${target} PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()
endfunction()

function(add_modtbl_dependency ex_name)
  set_source_files_properties(
    ${GENERATED_MODULE_HEADERS}
    PROPERTIES GENERATED TRUE
  )
  add_dependencies(${ex_name} parse_modules)
endfunction()
