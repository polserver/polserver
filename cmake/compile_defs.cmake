function(set_compile_flags target is_executable)
  message("* ${target}")
  target_include_directories(${target}  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} #own folder
    ${PROJECT_BINARY_DIR} #global config
  )
  
  # Add 'pol-core' to the include search path of all the projects in it
  if (${INCLUDE_POLCORE_DIR})    
    target_include_directories(${target} PRIVATE
    ${POLCORE_DIR} # path to pol-core
    )
  endif()

  target_include_directories(${target} SYSTEM PRIVATE
    ${BOOST_SOURCE_DIR} # boost
    "${POL_EXT_LIB_DIR}" #format/, antlr/, tinyxml/, etc
    "${POL_EXT_LIB_DIR}/picojson-1.3.0" #pico
    "${POL_EXT_LIB_DIR}/kaitai-runtime" #kaitai
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
      WINVER=0x0501
      _CONSOLE
    )
    if (${ARCH_BITS} EQUAL "64")
      target_compile_definitions(${target} PRIVATE
        _WIN64
      )
    endif()
  endif()
  
  target_compile_features(${target} PUBLIC cxx_std_17)

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
    >
  )

  if(${gcc} OR ${clang})
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
  # optional argument REUSE or REUSE_COTIRE
  # cmake starting with 3.16:
  # REUSE and REUSE_COTIRE will use clib pch
  # older cmake with cotire:
  # REUSE_COTIRE will create new pch with clib stdafx
  set(argn "${ARGN}")
  if (NOT NO_PCH)
    set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
    if (${CMAKE_VERSION} VERSION_LESS "3.16")
      if (NOT EXISTS ${_pch_name})
        if ("REUSE_COTIRE" STREQUAL "${argn}")
          set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/../clib/StdAfx.h")
        else()
          return()
        endif()
      endif()

      set_target_properties(${target} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${_pch_name})
      set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD OFF)
      cotire(${target})
      set_target_properties (clean_cotire PROPERTIES FOLDER 3rdParty)
      if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
        if(TARGET ${target}_pch)
          add_dependencies(${target}_pch boost)
        endif()
      endif()
    else()
      if ("${argn}" MATCHES "REUSE.*" AND REUSE_PCH)
        message("reuse clib")
        target_precompile_headers(${target} REUSE_FROM clib)
      else()
        if (NOT REUSE_PCH AND NOT EXISTS ${_pch_name})
          message("rebuild clib pch")
          set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/../clib/StdAfx.h")
        endif()
        message("build ${_pch_name}")
        target_precompile_headers(${target} PUBLIC ${_pch_name})
      endif()
    endif()
  endif()
endfunction()

function(use_curl target)
  target_include_directories(${target}
    PUBLIC ${CURL_INSTALL_DIR}/include
  )
  target_link_libraries(${target} PUBLIC ${CURL_LIB})
  if (NOT EXISTS ${CURL_LIB})
    add_dependencies(${target} libcurl)
  endif()
  target_compile_definitions(${target} PRIVATE
    CURL_STATICLIB
  )
  if (${linux})
    target_link_libraries(${target} PUBLIC ssl)
  else()
    target_link_libraries(${target} PUBLIC wldap32)
  endif()
endfunction()

function(use_benchmark target)
  if (ENABLE_BENCHMARK)
    target_link_libraries(${target} PUBLIC benchmark)
  endif()
endfunction()

function(use_zlib target)
    if(${windows})
      target_include_directories(${target}  PRIVATE
        "${POL_EXT_LIB_DIR}/zlib"
      )
    endif()
    target_link_libraries(${target} PRIVATE
      z
    )
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
