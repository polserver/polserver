add_library(libpicojson INTERFACE IMPORTED)
set_target_properties(libpicojson PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${POL_EXT_LIB_DIR}/picojson-1.3.0"
)
