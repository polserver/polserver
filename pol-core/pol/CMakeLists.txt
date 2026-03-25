set(ex_name pol)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)

add_modtbl_dependency(${ex_name})

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name})
use_benchmark(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  libcppdap
  plib
  libtinyxml
  libz
  libpicojson
  libcurl
  libsql
)

dist(${ex_name} .)

use_tidy(${ex_name})
