cmake_minimum_required(VERSION 3.2)

# protect the user for himself
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


project(polserver CXX C)

set(POL_VERSION_STR "100")
set(POL_VERSION 100)
set(POL_VERSION_NAME "Never Gonna Give You Up")

option(NO_PCH "Disable pre-compiled headers" OFF)
option(ENABLE_BENCHMARK "Enable benchmark support" OFF)

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

if(${ONLY_ECOMPILE} OR ${ONLY_RUNECL} OR ${ONLY_POL} OR ${ONLY_POLTOOL} OR ${ONLY_UOCONVERT} OR ${ONLY_UOTOOL})
  set(BUILD_ALL OFF)
endif()

include(cmake/init.cmake)

set(output_bin_dir "${CMAKE_SOURCE_DIR}/bin")

message("####################################")
message("#          ${PROJECT_NAME} - ${POL_VERSION_STR}         #")
message("# -   ${POL_VERSION_NAME}    - #")
message("####################################")
message("## CMake Version ${CMAKE_VERSION}")
message("## Generator ${CMAKE_GENERATOR} ${CMAKE_EXTRA_GENERATOR}")
message("## Output Dir: ${output_bin_dir}")
if (NO_PCH)
  message("## No precompiled header")
endif()
message("")

include(CheckIncludeFiles)
include(ExternalProject)


include(cmake/compile_defs.cmake)
include(cmake/release.cmake)
include(cmake/cotire.cmake)
#hide the cotire settings
hide_cotire()

fix_compiler_flags()
detect_compiler()
detect_arch()
detect_platform()
git_revision_target()

if (${linux})
  option(STRIP_BINARIES "Strip binaries directly, instead of only in package target" OFF)
endif()

prepare_build()
cmake_fake_target()

include(cmake/Boost.txt)
include(cmake/Benchmark.txt)
include(cmake/Format.txt)
include(cmake/Kaitai.txt)
if (${windows})
  include(cmake/ZLib.txt)
  include(cmake/StackWalker.txt)
endif()
if (BUILD_ALL OR ONLY_POL)
  include(cmake/Curl.txt)
endif()
include(cmake/TinyXML.txt)

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
