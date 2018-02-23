
macro(detect_compiler)
  if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING
		 "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	     FORCE
	   )
  endif()
  if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(debug 1)
	set(release 0)
  else()
	set(debug 0)
	set(release 1)
  endif()
  message("Build type: ${CMAKE_BUILD_TYPE} ${MinSizeRel}")
  set(clang 0)
  set(msvc 0)
  set(gcc 0)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(clang 1)
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(msvc 1)
    if (${MSVC_VERSION} VERSION_GREATER 1900)
      set(msvc_ide "15")
    else()
      set(msvc_ide "14")
    endif()
  elseif (CMAKE_COMPILER_IS_GNUCXX)
    set(gcc 1)
  endif()
  message("Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endmacro()

macro(detect_arch)
  if (NOT DEFINED FORCE_ARCH_BITS)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
      set(ARCH_BITS "64")
      set(ARCH_STRING "x64")
      message("Detected architecture is 64 bit")
    elseif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
      set(ARCH_BITS "32")
      set(ARCH_STRING "x86")
      message("Detected architecture is 32 bit")
    else()
      message("Unable to detect architecture of your system")
    endif()
    set(FORCE_ARCH_BITS 0)
  else()
    set(ARCH_BITS  ${FORCE_ARCH_BITS})
    if (${ARCH_BITS} EQUAL "64")
      set(ARCH_STRING "x64")
    else()
      set(ARCH_STRING "x86")
    endif()
    set(FORCE_ARCH_BITS 1)
    message("Architecture was explicitly defined as ${ARCH_BITS} bit")
  endif()
endmacro()

macro(detect_platform)
  set (linux 0)
  set (windows 0)
  if (UNIX AND NOT WIN32)
    message("Target system is Linux")
	set (linux 1)
  elseif (WIN32)
    set (windows 1)
  endif()
endmacro()

macro(prepare_build)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  SET(SOURCE_GROUP_DELIMITER "/")

  if(windows)
    set(HAVE_OPENSSL true)
    set(HAVE_MYSQL true)
  else()
    check_include_files (openssl/md5.h HAVE_OPENSSL)
    check_include_files (mysql/mysql.h HAVE_MYSQL)
  endif() 
  configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/env/pol_global_config.h.in 
    ${PROJECT_BINARY_DIR}/pol_global_config.h
  )
endmacro()

macro(get_git_revision)
  find_package(Git)
  if(GIT_EXECUTABLE)
	execute_process(COMMAND ${GIT_EXECUTABLE}
	  log -1 --pretty=format:%h
	  RESULT_VARIABLE status
	  OUTPUT_VARIABLE GIT_REVISION
	  #  ERROR_QUIET
	)
	if (status)
	  set(GIT_REVISION "\"1Unknown\"")
	  message(${status})
	  message(${GIT_REVISION})
	else()
	  set(GIT_REVISION "\"${GIT_REVISION}\"")
	endif()
  else()
	set(GIT_REVISION "\"2Unknown\"")
  endif()
endmacro()

macro(hide_cotire)
  mark_as_advanced(FORCE
	COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_EXTENSIONS
    COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_PATH
    COTIRE_DEBUG
    COTIRE_MAXIMUM_NUMBER_OF_UNITY_INCLUDES
    COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES
    COTIRE_UNITY_SOURCE_EXCLUDE_EXTENSIONS
    COTIRE_VERBOSE
  )
endmacro()

macro(cmake_fake_target)
  #used to have cmake files in VisualStudio as folder
  add_custom_target("cmakefiles"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
    SOURCES 
      cmake/init.cmake
      cmake/Benchmark.txt
      cmake/Boost.txt
      cmake/Curl.txt
      cmake/Format.txt
      cmake/release.cmake
      cmake/compile_defs.cmake
      cmake/env/pol_global_config.h.in
  )
  source_group(cmake FILES
    cmake/init.cmake
    cmake/Benchmark.txt
    cmake/Boost.txt
    cmake/Curl.txt
    cmake/Format.txt
    cmake/release.cmake
    cmake/compiler_defs.cmake
  )
  source_group(cmake/env FILES cmake/env/pol_global_config.h.in)
  set_target_properties(cmakefiles PROPERTIES EXCLUDE_FROM_ALL TRUE)
  set_target_properties(cmakefiles PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endmacro()
