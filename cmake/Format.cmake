#extra cmake file for format lib, just a simple one
#without all the extra stuff from the original one
#but better then having the cpp added to clib, like its was before

set(format_sources 
  ${POL_EXT_LIB_DIR}/format/format.cc 
  ${POL_EXT_LIB_DIR}/format/format.h)

set(lib_name format)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

if (${windows})
 target_compile_options(${lib_name} PRIVATE
   /wd4458 #var name hides
   /wd4456 #var name hides
  )
endif()

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
