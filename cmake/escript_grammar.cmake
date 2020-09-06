find_package(Java QUIET COMPONENTS Runtime)

if(NOT ANTLR_EXECUTABLE)
  find_program(ANTLR_EXECUTABLE
    NAMES antlr.jar antlr4.jar antlr-4.jar antlr-4.8-complete.jar
  )
endif()
if(Java_JAVA_EXECUTABLE AND ANTLR_EXECUTABLE)
  message("adding target 'escriptgrammar'")
  add_custom_target(escriptgrammar
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      -Dlanguage=Cpp -package EscriptGrammar
      ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.g4
      -o ${POL_EXT_LIB_DIR}/EscriptGrammar
    # You'll have to qualify the namespace antlr4::TokenStream
    # in the EscriptParser constructor when regenerating.
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      -Dlanguage=Cpp -visitor -package EscriptGrammar
      ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.g4
      -o ${POL_EXT_LIB_DIR}/EscriptGrammar
    COMMENT "Generating grammar files"
  )
endif()

#fake target for vstudio gui
add_custom_target(escriptfiles
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
  SOURCES 
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.g4
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.h
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.cpp
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.g4
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.h
    ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.cpp
)
source_group(escriptgrammar FILES
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.g4
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.h
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptParser.cpp
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.g4
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.h
  ${POL_EXT_LIB_DIR}/EscriptGrammar/EscriptLexer.cpp
)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_target_properties(escriptfiles PROPERTIES FOLDER Grammar)
