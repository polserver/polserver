function(set_compile_flags target is_executable)
  message("* ${target}")
  target_include_directories(${target}  PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} #own folder
    ${PROJECT_BINARY_DIR} #global config
    ${BOOST_SOURCE_DIR} # boost
    "${CMAKE_CURRENT_LIST_DIR}/../../lib" #format/..
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
  )
 
  if (${is_executable})
    if (${release} AND ${linux})
      target_link_libraries(${target} PUBLIC
        -s # strip
	  )
    endif()
	#search path for libs
	#    $<${linux}:
	#      -Wl,-R.
	#      -Wl,-R./lib
	#      -Wl,-R../lib
	#      -Wl,-R../../lib
	#    > 
  endif()

  if(${linux})
    link_directories(
      /usr/local/lib
      /usr/lib
      /usr/lib${ARCH_BITS}
    )
  endif()

  if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
    add_dependencies(${target} boost)
  endif()

  set_target_properties(${target} PROPERTIES
  	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/../bin"
  )

  #PCH SUPPORT
  set(_pch_name "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h")
  if (EXISTS ${_pch_name})
    set_target_properties(${target} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${_pch_name})
    cotire(${target})
  endif()

endfunction()

function(use_curl target)
  find_library(curl_lib NAMES curl libcurl
    PATHS ${CURL_INSTALL_DIR}/lib
    NO_DEFAULT_PATH)
 
  target_include_directories(${target}
    PUBLIC ${CURL_INSTALL_DIR}/include
  )
  if(curl_lib)
    target_link_libraries(${target} PUBLIC ${curl_lib})
  else()
    target_link_libraries(${target} PUBLIC curl)
  endif()
endfunction()

function(use_benchmark target)
  if (DEFINED ENABLE_BENCHMARK) 
    target_link_libraries(${target} PUBLIC benchmark)
  endif()
endfunction()

function(dist target dir)
  install(
    TARGETS ${target} 
    ARCHIVE DESTINATION ${dir}
    LIBRARY DESTINATION ${dir}
    RUNTIME DESTINATION ${dir}
    COMPONENT polcore
  )
endfunction()
