set(lib_name bscript)

include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)

set_compile_flags(${lib_name} 0)
warning_suppression(${lib_name})
enable_pch(${lib_name})

target_include_directories(${lib_name} PUBLIC ${ANTLR_INCLUDE_DIR})
if (NOT EXISTS ${ANTLR_LIB})
  add_dependencies(${lib_name} libantlr_ex)
endif()
target_compile_definitions(${lib_name} PUBLIC -DANTLR4CPP_STATIC)
          
target_link_libraries(${lib_name} PUBLIC
  ${ANTLR_LIB}
  clib
)
if (${linux})
  target_link_libraries(${lib_name} PUBLIC
    pthread    
  )
endif()

use_tidy(${lib_name})
