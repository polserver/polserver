set(ex_name uoconvert)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    
 
set_compile_flags(${ex_name} 1)

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
