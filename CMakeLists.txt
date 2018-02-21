cmake_minimum_required(VERSION 3.0)

# protect the user for himself
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
    FATAL: In-source builds are not allowed.
           Remove CMakeCache.txt and CMakeFiles folder and
           Switch to bin-build.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(polserver)

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

include(cmake/init.cmake)

if (NOT DEFINED GIT_REVISION)
  get_git_revision()
endif()

message("####################################")
message("#          ${PROJECT_NAME} - ${POL_VERSION_STR}         #")
message("# -   ${POL_VERSION_NAME}    - #")
message("# -           ${GIT_REVISION}          - #")
message("####################################")
message("## CMake Version ${CMAKE_VERSION}")
message("## Generator ${CMAKE_GENERATOR} ${CMAKE_EXTRA_GENERATOR}")
message("")

include(CheckIncludeFiles)
include(ExternalProject)


include(cmake/compile_defs.cmake)
include(cmake/release.cmake)
include(cmake/cotire.cmake)
#hide the cotire settings
hide_cotire()

detect_compiler()
detect_arch()
detect_platform()

prepare_build()
cmake_fake_target()

include(cmake/Boost.txt)
include(cmake/Benchmark.txt)
include(cmake/Format.txt)

if (BUILD_ALL OR ONLY_POL)
  include(cmake/Curl.txt)
endif()
add_subdirectory(lib/tinyxml)
add_subdirectory(pol-core)

#cmake package target for creating archive
release()
