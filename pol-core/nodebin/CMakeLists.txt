set(ex_name nodebin)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
)
 
set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name})

use_nodejs(${ex_name})

dist(${ex_name} .)

use_tidy(${ex_name})
