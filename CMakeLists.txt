cmake_minimum_required(VERSION 3.0)

project(polserver)

set(POL_VERSION_STR "099")
set(POL_VERSION 99)
set(POL_VERSION_NAME "Break Everything Even Rudder")

include(cmake/init.cmake)

if (NOT DEFINED GIT_REVISION)
  get_git_revision()
endif()

message("####################################")
message("#          ${PROJECT_NAME} - ${POL_VERSION_STR}         #")
message("# - ${POL_VERSION_NAME} - #")
message("# -           ${GIT_REVISION}          - #")
message("####################################")

include(CheckIncludeFiles)
include(ExternalProject)


include(cmake/utils.cmake)
include(cmake/release.cmake)
include(cmake/cotire.cmake)


detect_compiler()
detect_arch()
detect_platform()

prepare_build()

include(cmake/Boost.txt)
include(cmake/Curl.txt)
include(cmake/Benchmark.txt)

add_subdirectory(lib/tinyxml)
add_subdirectory(pol-core)

release()
