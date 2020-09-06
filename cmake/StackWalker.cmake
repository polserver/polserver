# windows only: build stackwalker

set(stackwalk_sources 
  ${POL_EXT_LIB_DIR}/StackWalker/StackWalker.cpp
  ${POL_EXT_LIB_DIR}/StackWalker/StackWalker.h
)
set(lib_name stackwalk)

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

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
