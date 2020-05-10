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
      ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.g4
      -o ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_EXECUTABLE}
      -Dlanguage=Cpp -visitor -package EscriptGrammar
      ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.g4
      -o ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar
    COMMENT "Generating grammar files"
  )
endif()

#fake target for vstudio gui
add_custom_target(escriptfiles
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
  SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.g4
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.h
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.g4
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.h
    ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.cpp
)
source_group(escriptgrammar FILES
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.g4
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptParser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.g4
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.h
  ${CMAKE_CURRENT_LIST_DIR}/../lib/EscriptGrammar/EscriptLexer.cpp
)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(escriptfiles PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_target_properties(escriptfiles PROPERTIES FOLDER Grammar)
