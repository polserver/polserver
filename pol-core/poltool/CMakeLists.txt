set(ex_name poltool)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  clib
  plib
  libz
)
dist(${ex_name} .)

use_tidy(${ex_name})
enable_pch(${ex_name} REUSE)
