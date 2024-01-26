add_library(libutf8 INTERFACE IMPORTED)
set_target_properties(libutf8 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${POL_EXT_LIB_DIR}/utf8-2.3.4"
)
