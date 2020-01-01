set(ex_name ecompile)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    
 
set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib 
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    pthread    
  )
endif()
dist(${ex_name} scripts)

use_tidy(${ex_name})
enable_pch(${ex_name} REUSE)
