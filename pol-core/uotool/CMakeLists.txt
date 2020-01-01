set(ex_name uotool)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name} REUSE_COTIRE)

target_link_libraries(${ex_name} PUBLIC
  clib
  plib
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    pthread    
  )
endif()

dist(${ex_name} .)

use_tidy(${ex_name})
