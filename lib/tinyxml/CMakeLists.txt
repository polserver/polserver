set(lib_name tinyxml)

add_library(${lib_name} STATIC #SHARED
  # sorted !
  tinystr.cpp 
  tinyxml.cpp
  tinyxmlerror.cpp 
  tinyxmlparser.cpp 
)

set_compile_flags(${lib_name} 0)

target_compile_definitions(${lib_name} PUBLIC
  TIXML_USE_STL
)
