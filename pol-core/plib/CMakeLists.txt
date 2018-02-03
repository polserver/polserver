set(lib_name plib)

include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)

set_compile_flags(${lib_name} 0)

target_link_libraries(${lib_name} PUBLIC
  clib
)

