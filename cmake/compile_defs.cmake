function(set_compile_flags target is_executable)
  message("* ${target}")
  target_include_directories(${target}  PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} #own folder
    ${PROJECT_BINARY_DIR} #global config
    ${BOOST_SOURCE_DIR} # boost
    "${CMAKE_CURRENT_LIST_DIR}/../../lib" #format/..
    "${CMAKE_CURRENT_LIST_DIR}/../../lib/picojson-1.3.0" #pico
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
  endif()

  target_compile_options(${target} PRIVATE
	$<${linux}:
      -std=c++11
      -fPIC
      -W
      -Wall
      -Wno-unknown-pragmas
      -Wno-unused-result
      -Wno-unused-function
      -Wno-format
      -fno-strict-aliasing  
	>

    $<$<AND:${FORCE_ARCH_BITS},${linux}>:
      -m${ARCH_BITS}
    >
	
	$<$<AND:${debug},${linux}>:
	  -ggdb
	>

	$<${gcc}:
      -fopenmp
	>

	$<${windows}:
	  /MT # runtime lib
      /MP # multiprocess build
	  /openmp
	>
  )
 
  if (${is_executable})
    if (${release} AND ${linux})
      target_link_libraries(${target} PUBLIC
        -s # strip
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
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
  )

  source_group_by_folder(${target})
  #ist
  #release-static\include" /I"C:\Tools\vcpkg\installed\x64-windows\include" /nologo /W3 /WX- /diagnostics:classic /O2 /Ob2 /D WIN32 /D _WINDOWS /D NDEBUG /D INC_PASSERT=1 /D __STDC_CONSTANT_MACROS /D __STDC_FORMAT_MACROS /D __STDC_LIMIT_MACROS /D _REENTRANT /D ARCH_BITS=64 /D RELEASE_VERSION /D _WIN32 /D NOMINMAX /D WINDOWS /D WINVER=0x0501 /D TIXML_USE_STL /D "CMAKE_INTDIR=\"Release\"" /D _MBCS /Gm- /EHsc /MD /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /GR /Yc"C:\projects\pol_testing\pol-core\bin-build\pol-core\pol\Release\cotire\pol_CXX_prefix.hxx" /Fp"C:\projects\pol_testing\pol-core\bin-build\pol-core\pol\Release\cotire\pol_CXX_prefix.pch" /Fo"pol.dir\Release\\" /Fd"pol.dir\Release\vc141.pdb" /Gd /TP  /errorReport:queue  /Zm170
  
  #  Release 64 bit pol compflag:
  # /MP /GS- /W4 /Gy /Zc:wchar_t  /Zi /Gm- /O2 /Ob2 /Fd".\Release/x64/"  /Zc:inline  /D "_CONSOLE"   /D "HAVE_MYSQL" /D "_MBCS" /errorReport:prompt /GF /WX- /Zc:forScope /Gd /MT /openmp /Fa".\Release/x64/" /EHa /nologo /Fo".\Release/x64/"

  #  Release 64bit linker pol:
  #  /OUT:".\Release/x64/pol.exe" /MANIFEST /PDB:".\Release/x64/pol.pdb" /DYNAMICBASE:NO "normaliz.lib" "wsock32.lib" "odbc32.lib" "odbccp32.lib" "libmysql.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" /LARGEADDRESSAWARE /DEBUG /MACHINE:X64 /OPT:REF /INCREMENTAL:NO /PGD:".\Release\x64/pol.pgd" /SUBSYSTEM:CONSOLE",5.02" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:".\Release\x64/pol.exe.intermediate.manifest" /MAP":.\Release/x64/pol.map" /ERRORREPORT:PROMPT /NOLOGO  /TLBID:1 
endfunction()

function(source_group_by_folder target)
  set(last_dir "")
  set(files "")
  foreach(file ${${target}_sources})
    get_filename_component(dir "${file}" PATH)
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
  if (NOT NO_PCH)
    set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
    if (EXISTS ${_pch_name})
      set_target_properties(${target} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${_pch_name})
      set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD OFF)
      cotire(${target})
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
endfunction()

function(use_benchmark target)
  if (ENABLE_BENCHMARK) 
    target_link_libraries(${target} PUBLIC benchmark)
  endif()
endfunction()

