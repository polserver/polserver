set(escriptgrammarlib_sources 
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.cpp
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.h
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.cpp
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.h
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParserBaseListener.cpp
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParserBaseListener.h
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParserListener.cpp
${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParserListener.h
)
 
set(lib_name escriptgrammarlib)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

target_include_directories(${lib_name} PRIVATE
  "${POL_EXT_LIB_DIR}/EscriptGrammar"
)
target_link_libraries(${lib_name} PUBLIC
  libantlr
)

warning_suppression(${lib_name})

target_compile_options(${lib_name} PRIVATE
    $<${windows}:
      /wd4100 #unreferenced formal parameter
    >
    $<${linux}:
      -Wno-unused-parameter
      -Wno-attributes
    >
)

set_target_properties (${lib_name} PROPERTIES FOLDER 3rdParty)
