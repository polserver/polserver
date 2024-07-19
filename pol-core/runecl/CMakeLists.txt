set(ex_name runecl)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)

add_modtbl_dependency(${ex_name})

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name} REUSE_COTIRE)

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  libtinyxml
  libpicojson
)

dist(${ex_name} scripts)

use_tidy(${ex_name})
