set(lib_name libtinyxml)
set(tinydir ${POL_EXT_LIB_DIR}/tinyxml-2.6.2)

add_library(${lib_name} STATIC
  EXCLUDE_FROM_ALL
  # sorted !
  ${tinydir}/tinyxml/tinystr.cpp 
  ${tinydir}/tinyxml/tinyxml.cpp
  ${tinydir}/tinyxml/tinyxmlerror.cpp 
  ${tinydir}/tinyxml/tinyxmlparser.cpp 
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

set_target_properties (${lib_name} PROPERTIES 
  INTERFACE_INCLUDE_DIRECTORIES ${tinydir}
  FOLDER 3rdParty
)
