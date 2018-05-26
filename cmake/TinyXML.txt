set(lib_name tinyxml)

add_library(${lib_name} STATIC
  # sorted !
  ${CMAKE_CURRENT_LIST_DIR}/../lib/tinyxml/tinystr.cpp 
  ${CMAKE_CURRENT_LIST_DIR}/../lib/tinyxml/tinyxml.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../lib/tinyxml/tinyxmlerror.cpp 
  ${CMAKE_CURRENT_LIST_DIR}/../lib/tinyxml/tinyxmlparser.cpp 
)

set_compile_flags(${lib_name} 0)

target_compile_definitions(${lib_name} PUBLIC
  TIXML_USE_STL
)

if (${windows})
 target_compile_options(${lib_name} PRIVATE
   /wd4458 #var name hides
  )
endif()

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
