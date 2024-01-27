
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

macro(test_file_offset)
  check_cxx_source_compiles(
    "#include <sys/types.h>
#define KB ((off_t)1024)
#define MB ((off_t)1024 * KB)
#define GB ((off_t)1024 * MB)
#define TB ((off_t)1024 * GB)
int t2[(((64 * GB -1) % 671088649) == 268434537)
       && (((TB - (64 * GB -1) + 255) % 1792151290) == 305159546)? 1: -1];

int main()
{
  return 0;
}"
    file_offset_test
  )
endmacro()

macro(detect_platform)
  set (linux 0)
  set (windows 0)
  set (arm_proc 0)
  if (UNIX AND NOT WIN32)
    set (linux 1)
  elseif (WIN32)
    set (windows 1)
  endif()

  test_big_endian(bigendian)
  if (bigendian)
    #Error? I don't think that it really works
    message("Platform is Big Endian")
  else()
    message("Platform is Little Endian")
  endif()

  message("Compiling on processor ${CMAKE_SYSTEM_PROCESSOR}")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
    set(arm_proc 1)
  endif()

  # Needed by ExternalProject to add list arguments
  string(REPLACE ";" "|" PIPED_OSX_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}")

  if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    message("  macOS architectures: ${CMAKE_OSX_ARCHITECTURES}")
  endif()

  #atleast a problem when arm (32bit) processor runs in a VM
  set(file_offset_bits 0)
  if (linux)
    test_file_offset()
    if (NOT file_offset_test)
      set(CMAKE_REQUIRED_DEFINITIONS -D_FILE_OFFSET_BITS=64)
      test_file_offset()
      set(CMAKE_REQUIRED_DEFINITIONS )
      if (file_offset_test)
        set(file_offset_bits 1)
        message("Using 64bit file interface")
      endif()
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
  elseif (APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(OPENSSL openssl)
    set(HAVE_OPENSSL ${OPENSSL_FOUND})
    pkg_search_module(MYSQL mysqlclient)
    set(HAVE_MYSQL ${MYSQL_FOUND})
    check_include_files (zlib.h HAVE_ZLIB)
  else()
    check_include_files (openssl/evp.h HAVE_OPENSSL)
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
    message(WARNING "MySQL not found") 
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
      cmake/Antlr.cmake
      cmake/Benchmark.cmake
      cmake/Boost.cmake
      cmake/Cppdap.cmake
      cmake/Curl.cmake
      cmake/EscriptGrammarLib.cmake
      cmake/escript_grammar.cmake
      cmake/Fmt.cmake
      cmake/Kaitai.cmake
      cmake/PicoJson.cmake
      cmake/SQL.cmake
      cmake/StackWalker.cmake
      cmake/TinyXML.cmake
      cmake/UTF8.cmake
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
    cmake/Antlr.cmake
    cmake/Benchmark.cmake
    cmake/Boost.cmake
    cmake/Cppdap.cmake
    cmake/Curl.cmake
    cmake/EscriptGrammarLib.cmake
    cmake/escript_grammar.cmake
    cmake/Fmt.cmake
    cmake/Kaitai.cmake
    cmake/PicoJson.cmake
    cmake/SQL.cmake
    cmake/StackWalker.cmake
    cmake/TinyXML.cmake
    cmake/UTF8.cmake
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
  if (NOT GIT_EXECUTABLE)
    message(WARNING "Git not found unable to store revision")
  endif()
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
