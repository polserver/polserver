# windows only: build stackwalker

set(libstackwalk_sources 
  ${POL_EXT_LIB_DIR}/StackWalker/StackWalker.cpp
  ${POL_EXT_LIB_DIR}/StackWalker/StackWalker.h
)
set(lib_name libstackwalk)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

if (${windows})
 target_compile_options(${lib_name} PRIVATE
   /wd4091 #typedef
   /wd4996 #deprecated
  )
endif()

set_target_properties (${lib_name} PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${POL_EXT_LIB_DIR}
  FOLDER 3rdParty
)
