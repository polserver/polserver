# windows only: build zlib
set(z_sources 
  ${POL_EXT_LIB_DIR}/zlib/adler32.c
  ${POL_EXT_LIB_DIR}/zlib/compress.c
  ${POL_EXT_LIB_DIR}/zlib/crc32.c
  ${POL_EXT_LIB_DIR}/zlib/crc32.h
  ${POL_EXT_LIB_DIR}/zlib/deflate.c
  ${POL_EXT_LIB_DIR}/zlib/deflate.h
  ${POL_EXT_LIB_DIR}/zlib/inffast.c
  ${POL_EXT_LIB_DIR}/zlib/inffast.h
  ${POL_EXT_LIB_DIR}/zlib/inffixed.h
  ${POL_EXT_LIB_DIR}/zlib/inflate.c
  ${POL_EXT_LIB_DIR}/zlib/inflate.h
  ${POL_EXT_LIB_DIR}/zlib/inftrees.c
  ${POL_EXT_LIB_DIR}/zlib/inftrees.h
  ${POL_EXT_LIB_DIR}/zlib/trees.c
  ${POL_EXT_LIB_DIR}/zlib/trees.h
  ${POL_EXT_LIB_DIR}/zlib/uncompr.c
  ${POL_EXT_LIB_DIR}/zlib/zconf.h
  ${POL_EXT_LIB_DIR}/zlib/zlib.h
  ${POL_EXT_LIB_DIR}/zlib/zutil.c
  ${POL_EXT_LIB_DIR}/zlib/zutil.h
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
