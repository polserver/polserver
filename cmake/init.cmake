
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
  message("Build type: ${CMAKE_BUILD_TYPE}")
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
    set (linux 1)
  elseif (WIN32)
    set (windows 1)
  endif()
  include(TestBigEndian)
  test_big_endian(bigendian)
  if (bigendian)
    message("Platform is Big Endian")
  else()
    message("Platform is Little Endian")
  endif()

  if (NOT DEFINED FORCE_SIGNED_CHAR)
    set(FORCE_SIGNED_CHAR 0)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles(
      "#include <type_traits>
      int main()
      {
        static_assert(std::is_signed<char>::value, \"char is unsigned\");
        return 0;
      }"
      CHAR_IS_SIGNED
    )
#    if (NOT CHAR_IS_SIGNED)
#      message("char is unsigned, forcing as signed")
#      set(FORCE_SIGNED_CHAR 1)
#    endif()
  endif()
  include(CheckTypeSize)
  set(CMAKE_EXTRA_INCLUDE_FILES wchar.h)
  check_type_size(wchar_t SIZEOF_WCHAR_T)
  set(CMAKE_EXTRA_INCLUDE_FILES)
  message("wchar size is ${SIZEOF_WCHAR_T}")

  set(arm 0)
  if (${linux})
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
      set(arm 1)
      message("Platform is ARM")
    endif()
  endif()

endmacro()

macro(fix_compiler_flags)
  #remove default flags which collide with out settings
  set(variables
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
  )
  foreach(variable ${variables})
    if(${variable} MATCHES "/MDd")
      string(REGEX REPLACE "/MDd" "" ${variable} "${${variable}}")
    endif()
    if(${variable} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "" ${variable} "${${variable}}")
    endif()
    if(${variable} MATCHES "/EHsc")
      string(REGEX REPLACE "/EHsc" "" ${variable} "${${variable}}")
    endif()
    if(${variable} MATCHES "/EHs")
      string(REGEX REPLACE "/EHs" "" ${variable} "${${variable}}")
    endif()
    if(${variable} MATCHES "/W3")
      string(REGEX REPLACE "/W3" "" ${variable} "${${variable}}")
    endif()
  endforeach()
endmacro()

macro(prepare_build)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # a few external tools need it
  set(SOURCE_GROUP_DELIMITER "/")

  if(windows)
    set(HAVE_OPENSSL true)
    set(HAVE_MYSQL true)
    set(HAVE_ZLIB true)
  else()
    check_include_files (openssl/md5.h HAVE_OPENSSL)
    check_include_files (mysql/mysql.h HAVE_MYSQL)
    check_include_files (zlib.h HAVE_ZLIB)
  endif()
  if (NOT HAVE_OPENSSL)
    message(FATAL_ERROR "OpenSSL not found")
  endif()
  if (NOT HAVE_ZLIB)
    message(FATAL_ERROR "ZLib not found")
  endif()
  if (NOT HAVE_MYSQL)
    message("MySQL not found")
  endif()
  configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/env/pol_global_config.h.in 
    ${PROJECT_BINARY_DIR}/pol_global_config.h
  )
  #building with ninja disables color without explicit forcing it
  if(gcc)
    add_compile_options(-fdiagnostics-color=always)
  elseif(clang)
    add_compile_options(-fcolor-diagnostics)
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
      cmake/Benchmark.cmake
      cmake/Boost.cmake
      cmake/Curl.cmake
      cmake/Format.cmake
      cmake/Kaitai.cmake
      cmake/StackWalker.cmake
      cmake/TinyXML.cmake
      cmake/ZLib.cmake
      cmake/release.cmake
      cmake/git_revision.cmake
      cmake/compile_defs.cmake
      cmake/c_flag_overrides.cmake
      cmake/cxx_flag_overrides.cmake
      cmake/env/pol_global_config.h.in
      cmake/env/pol_revision.h.in
  )
  source_group(cmake FILES
    cmake/init.cmake
    cmake/Benchmark.cmake
    cmake/Boost.cmake
    cmake/Curl.cmake
    cmake/Format.cmake
    cmake/Kaitai.cmake
    cmake/StackWalker.cmake
    cmake/TinyXML.cmake
    cmake/ZLib.cmake
    cmake/release.cmake
    cmake/git_revision.cmake
    cmake/compile_defs.cmake
  )
  source_group(cmake/env FILES 
    cmake/env/pol_global_config.h.in
    cmake/env/pol_revision.h.in
  )
  set_target_properties(cmakefiles PROPERTIES EXCLUDE_FROM_ALL TRUE)
  set_target_properties(cmakefiles PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  set_target_properties(cmakefiles PROPERTIES FOLDER Misc)
endmacro()

macro(git_revision_target)
  find_package(Git)
  add_custom_target(git_rev
    COMMAND ${CMAKE_COMMAND}
    -DGIT=${GIT_EXECUTABLE}
    -DCFG_FILE=${CMAKE_CURRENT_LIST_DIR}/cmake/env/pol_revision.h.in 
    -DTMP_FILE=${PROJECT_BINARY_DIR}/pol_revision.h.tmp
    -DOUT_FILE=${PROJECT_BINARY_DIR}/pol_revision.h
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/git_revision.cmake
    BYPRODUCTS=${PROJECT_BINARY_DIR}/pol_revision.h 
  )
  set_target_properties(git_rev PROPERTIES FOLDER Misc)
endmacro()
