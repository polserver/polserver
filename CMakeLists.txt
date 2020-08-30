cmake_minimum_required(VERSION 3.9)

#protect the user from himself
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
    FATAL: In-source builds are not allowed.
           Remove CMakeCache.txt and CMakeFiles folder and
           Switch to bin-build.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "!BuildTargets")

#fix default compiler flags for MSVC (/Md vs /MT)
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)


project(polserver 
	LANGUAGES CXX C
	VERSION 100.0.0
	DESCRIPTION "Penultima Online"
	HOMEPAGE_URL "polserver.com")

set(POL_VERSION_STR "${polserver_VERSION}")
set(POL_VERSION_NAME "Never Gonna Give You Up")
set(POL_COPYRIGHT "Copyright (C) 1993-2020 Eric N. Swanson")

option(NO_PCH "Disable pre-compiled headers" OFF)
option(USE_CCACHE "Use ccache if found" OFF)
option(ENABLE_BENCHMARK "Enable benchmark support" OFF)
option(ENABLE_FLYWEIGHT_REPORT "Enables flyweight memory report in memoryusage.log" OFF)
option(GCOV "Build for coverage" OFF)

option(BUILD_ALL "Build everything" ON)
option(ONLY_ECOMPILE "Build only ecompile" OFF)
option(ONLY_RUNECL "Build only runecl" OFF)
option(ONLY_POL "Build only pol" OFF)
option(ONLY_POLTOOL "Build only poltool" OFF)
option(ONLY_UOCONVERT "Build only uoconvert" OFF)
option(ONLY_UOTOOL "Build only uotool" OFF)

option(ENABLE_ASAN "Enables Address sanitizer" OFF)
option(ENABLE_USAN "Enables Undefined sanitizer" OFF)
option(ENABLE_MSAN "Enables Memory sanitizer" OFF)
option(ENABLE_TSAN "Enables Thread sanitizer" OFF)

option(ENABLE_TIDY "Enables clang tidy check" OFF)
#e.g "-fix;-checks=modernize-use-nullptr"
set(TIDY_ARGS "" CACHE STRING "clang tidy arguments")

option(REUSE_PCH "Reuse clib pch if needed (windows only)" OFF)

if(${ONLY_ECOMPILE} OR ${ONLY_RUNECL} OR ${ONLY_POL} OR ${ONLY_POLTOOL} OR ${ONLY_UOCONVERT} OR ${ONLY_UOTOOL})
  set(BUILD_ALL OFF)
endif()
if(${ENABLE_TIDY})
  set(NO_PCH ON)
endif()


include(CheckIncludeFiles)
include(ExternalProject)
include(CheckCXXSourceCompiles)
include(TestBigEndian)

include(cmake/init.cmake)

set(POL_EXT_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(output_bin_dir "${CMAKE_SOURCE_DIR}/bin")

message("####################################")
message("## ${PROJECT_NAME} - ${POL_VERSION_STR}")
message("## ${POL_VERSION_NAME}")
message("####################################")
message("## CMake Version ${CMAKE_VERSION}")
message("## Generator ${CMAKE_GENERATOR} ${CMAKE_EXTRA_GENERATOR}")
message("## Output Dir: ${output_bin_dir}")
if (NO_PCH)
  message("## No precompiled header")
endif()

if (ENABLE_TIDY)
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
  )
  if(NOT CLANG_TIDY_EXE)
    message(ERROR "clang-tidy not found.")
  else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    message(STATUS "clang-tidy args: ${TIDY_ARGS}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "${TIDY_ARGS}")
  endif()
endif()

if(USE_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message("## building with ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
endif()

message("")

include(cmake/compile_defs.cmake)
include(cmake/release.cmake)
if (${CMAKE_VERSION} VERSION_LESS "3.16")
  include(cmake/cotire.cmake)
  #hide the cotire settings
  hide_cotire()
endif()

fix_compiler_flags()
detect_compiler()
detect_arch()
detect_platform()
git_revision_target()

if (${linux})
  option(STRIP_BINARIES "Strip binaries directly, instead of only in package target" OFF)
  
  set(REUSE_PCH ON)
endif()

prepare_build()
cmake_fake_target()

include(cmake/Antlr.cmake)
include(cmake/Boost.cmake)
include(cmake/Benchmark.cmake)
include(cmake/escript_grammar.cmake)
include(cmake/Format.cmake)
include(cmake/Kaitai.cmake)
if (${windows})
  include(cmake/ZLib.cmake)
  include(cmake/StackWalker.cmake)
endif()
if (BUILD_ALL OR ONLY_POL)
  include(cmake/Curl.cmake)
endif()
include(cmake/TinyXML.cmake)

add_subdirectory(docs)
add_subdirectory(pol-core)

#cmake package target for creating archive
release()

if (BUILD_ALL OR ONLY_POL)
  if (${CMAKE_VERSION} VERSION_GREATER "3.6.0")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pol)
  endif()
endif()

add_polrelease_target()

#ctest integration
if (BUILD_ALL OR (ONLY_ECOMPILE AND ONLY_RUNECL) OR (ONLY_POLTOOL AND ONLY_UOCONVERT))
  message("activating test target")
  enable_testing()

  #splitted into each escript testfolder to allow parallel execution
  if (BUILD_ALL OR (ONLY_ECOMPILE AND ONLY_RUNECL))
    set(testdir ${CMAKE_CURRENT_SOURCE_DIR}/testsuite/escript)
    file(GLOB children RELATIVE ${testdir} ${testdir}/*)

    foreach(child ${children})
      if(IS_DIRECTORY ${testdir}/${child})
        string(FIND "${child}" "_" out)
        if(${out} EQUAL 0)
          continue()
        endif()
        add_test(NAME escript_${child}
          COMMAND ${CMAKE_COMMAND}
            -Dtestdir=${testdir}
            -Dsubtest=${child}
            -Decompile=${output_bin_dir}/ecompile
            -Drunecl=${output_bin_dir}/runecl
            -Derrfilesuffix=1
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/escripttest.cmake
          WORKING_DIRECTORY ${testdir}
        )
      endif()
    endforeach()
    add_test(NAME escript_compiler_parity
            COMMAND ${output_bin_dir}/ecompile -f -G
            WORKING_DIRECTORY ${testdir}
            )
  endif()
  if (BUILD_ALL OR (ONLY_POLTOOL AND ONLY_UOCONVERT))
    include(cmake/core_tests.cmake)
  endif()
endif()

#clang-format target for modified and new files
find_program(
  CLANG_FORMAT_EXE
    NAMES "clang-format"
    DOC "Path to clang-format executable"
  )
find_package(Git)
if(CLANG_FORMAT_EXE AND GIT_EXECUTABLE)
  message("adding target 'clang_format'")
  add_custom_target(clang_format
    COMMAND ${CMAKE_COMMAND}
      -DGIT=${GIT_EXECUTABLE}
      -DCLANG_FORMAT=${CLANG_FORMAT_EXE}
      -DWORKDIR=${CMAKE_CURRENT_SOURCE_DIR}/pol-core
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang_format.cmake
    COMMENT "Formating modified files"
  )
  set_target_properties(clang_format PROPERTIES EXCLUDE_FROM_ALL TRUE)
  set_target_properties(clang_format PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  set_target_properties(clang_format PROPERTIES FOLDER !BuildTargets)
else()
  message("Git or clang-format not found for target 'clang_format'")
endif()

