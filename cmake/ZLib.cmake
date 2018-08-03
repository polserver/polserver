# windows only: build zlib
set(z_sources 
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/adler32.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/compress.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/crc32.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/crc32.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/deflate.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/deflate.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inffast.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inffast.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inffixed.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inflate.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inflate.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inftrees.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/inftrees.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/trees.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/trees.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/uncompr.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/zconf.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/zlib.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/zutil.c
  ${CMAKE_CURRENT_LIST_DIR}/../lib/zlib/zutil.h
)

set(lib_name z)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

target_compile_options(${lib_name} PRIVATE
  /wd4131 # old-style decorator
  /wd4244 # conversion loss of data
  /wd4127 # conditional expression is constant
)

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
