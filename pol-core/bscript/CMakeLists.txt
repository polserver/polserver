set(lib_name bscript)

include(CMakeSources.cmake)

set(module_byproducts "")
file(GLOB ems  "${CMAKE_SOURCE_DIR}/pol-core/support/scripts/*.em")
foreach(em ${ems})
  get_filename_component(em_name ${em} NAME_WE)
  list(APPEND module_byproducts "${PROJECT_BINARY_DIR}/modules/${em_name}-tbl.h")
endforeach()
# message("byproducts!! ${module_byproducts}")

add_custom_target(parse_modules
    COMMAND ${CMAKE_COMMAND}
    -DEM_FOLDER=${CMAKE_SOURCE_DIR}/pol-core/support/scripts
    -DOUT_FOLDER=${PROJECT_BINARY_DIR}/modules/
    -P ${CMAKE_SOURCE_DIR}/cmake/module_parse.cmake
    BYPRODUCTS ${PROJECT_BINARY_DIR}/modules/modules.cpp
)
set_target_properties(parse_modules PROPERTIES FOLDER Misc)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)

add_dependencies(bscript parse_modules)

set_compile_flags(${lib_name} 0)
warning_suppression(${lib_name})
enable_pch(${lib_name})

target_link_libraries(${lib_name} PUBLIC
  clib
)
if (${linux})
  target_link_libraries(${lib_name} PUBLIC
    pthread    
  )
endif()

use_tidy(${lib_name})
