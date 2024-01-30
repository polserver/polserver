set(grammardir ${POL_EXT_LIB_DIR}/Parser)

set(libescriptgrammar_sources
${grammardir}/EscriptGrammar/EscriptLexer.cpp
${grammardir}/EscriptGrammar/EscriptLexer.h
${grammardir}/EscriptGrammar/EscriptParser.cpp
${grammardir}/EscriptGrammar/EscriptParser.h
${grammardir}/EscriptGrammar/EscriptParserBaseListener.cpp
${grammardir}/EscriptGrammar/EscriptParserBaseListener.h
${grammardir}/EscriptGrammar/EscriptParserListener.cpp
${grammardir}/EscriptGrammar/EscriptParserListener.h
)
 
set(lib_name libescriptgrammar)

add_library(${lib_name} STATIC
  EXCLUDE_FROM_ALL
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

target_include_directories(${lib_name} PRIVATE
  "${grammardir}/EscriptGrammar"
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

set_target_properties (${lib_name} PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${grammardir}
  FOLDER 3rdParty
)
