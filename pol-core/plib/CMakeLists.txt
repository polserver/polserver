set(lib_name plib)

include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)

set_compile_flags(${lib_name} 0)
warning_suppression(${lib_name})
enable_pch(${lib_name})

target_link_libraries(${lib_name} PUBLIC
  clib
  libkaitai
)
if(${linux})
  set_source_files_properties(uopreader/uop.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
endif()
if(${windows})
  set_source_files_properties(uopreader/uop.cpp PROPERTIES COMPILE_FLAGS /wd4100)
endif()

use_tidy(${lib_name})
