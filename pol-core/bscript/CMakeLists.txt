set(lib_name bscript)

include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)

set_compile_flags(${lib_name} 0)
warning_suppression(${lib_name})
enable_pch(${lib_name})

target_link_libraries(${lib_name} PUBLIC
  clib
  escriptgrammarlib
)

use_tidy(${lib_name})
