set(kaitai_sources 
  ${CMAKE_CURRENT_LIST_DIR}/../lib/kaitai-runtime/kaitai/custom_decoder.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/kaitai-runtime/kaitai/kaitaistream.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../lib/kaitai-runtime/kaitai/kaitaistream.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/kaitai-runtime/kaitai/kaitaistruct.h
)
 
set(lib_name kaitai)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

target_include_directories(${lib_name}  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../lib/kaitai-runtime"
)
if(${windows})
  target_include_directories(${lib_name}  PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../lib/zlib"
  )
endif()
target_link_libraries(${lib_name} PRIVATE
  z
)
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
