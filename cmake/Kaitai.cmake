set(kaitai_sources 
  ${POL_EXT_LIB_DIR}/kaitai-runtime/kaitai/custom_decoder.h
  ${POL_EXT_LIB_DIR}/kaitai-runtime/kaitai/kaitaistream.cpp
  ${POL_EXT_LIB_DIR}/kaitai-runtime/kaitai/kaitaistream.h
  ${POL_EXT_LIB_DIR}/kaitai-runtime/kaitai/kaitaistruct.h
)
 
set(lib_name kaitai)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

target_include_directories(${lib_name}  PRIVATE
  "${POL_EXT_LIB_DIR}/kaitai-runtime"
)

use_zlib(${lib_name})
warning_suppression(${lib_name})

target_compile_definitions(${lib_name} PRIVATE
  KS_STR_ENCODING_NONE
)
target_compile_definitions(${lib_name} PUBLIC
  KS_ZLIB
)
if(${linux})
  target_compile_options(${lib_name} PRIVATE
    -Wno-sign-compare
    -Wno-unused-parameter
    -fno-strict-aliasing
  )
endif()
if (${windows})
 target_compile_options(${lib_name} PRIVATE
   /wd4458 #var name hides
   /wd4456 #var name hides
   /wd4267 #convert types
  )
endif()

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
