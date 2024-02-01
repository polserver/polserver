find_package(Java QUIET COMPONENTS Runtime)

get_target_property (grammarbasedir libescriptgrammar
  INTERFACE_INCLUDE_DIRECTORIES)
set(grammardir ${grammarbasedir}/EscriptGrammar)

if(NOT ANTLR_EXECUTABLE)
  find_program(ANTLR_EXECUTABLE
    NAMES antlr.jar antlr4.jar antlr-4.jar antlr-4.13.1-complete.jar
  )
endif()
if(Java_JAVA_EXECUTABLE AND ANTLR_EXECUTABLE)
  message("adding target 'escriptgrammar_codegen'")
  add_custom_target(escriptgrammar_codegen
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      -Dlanguage=Cpp -package EscriptGrammar
      ${grammardir}/EscriptLexer.g4
      -o ${grammardir}
    # You'll have to qualify the namespace antlr4::TokenStream
    # in the EscriptParser constructor when regenerating.
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      -Dlanguage=Cpp -visitor -package EscriptGrammar
      ${grammardir}/EscriptParser.g4
      -o ${grammardir}
    COMMENT "Generating grammar files"
  )
endif()

#fake target for vstudio gui
add_custom_target(escriptfiles
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
  SOURCES 
    ${grammardir}/EscriptParser.g4
    ${grammardir}/EscriptParser.h
    ${grammardir}/EscriptParser.cpp
    ${grammardir}/EscriptLexer.g4
    ${grammardir}/EscriptLexer.h
    ${grammardir}/EscriptLexer.cpp
)
source_group(escriptgrammar FILES
  ${grammardir}/EscriptParser.g4
  ${grammardir}/EscriptParser.h
  ${grammardir}/EscriptParser.cpp
  ${grammardir}/EscriptLexer.g4
  ${grammardir}/EscriptLexer.h
  ${grammardir}/EscriptLexer.cpp
)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_target_properties(escriptfiles PROPERTIES FOLDER Grammar)
