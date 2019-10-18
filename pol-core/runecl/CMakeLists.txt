set(ex_name runecl)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)

set_source_files_properties(
  ${PROJECT_BINARY_DIR}/modtbl-runecl.cpp
  PROPERTIES GENERATED TRUE
)

add_dependencies(${ex_name} parse_modules)

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name} REUSE_COTIRE)

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  tinyxml
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    pthread    
  )
endif()

dist(${ex_name} scripts)

use_tidy(${ex_name})
