find_package(Java QUIET COMPONENTS Runtime)

if(CMAKE_SCRIPT_MODE_FILE)
  file(GLOB files "${path}/*.cpp" "${path}/*.h" )
  foreach(file ${files})
    get_filename_component(filename "${file}" NAME)
    message("* Fixing ${filename}")
    file(READ ${file} contents)
    string(REGEX REPLACE "Generated from [^\r\n]*lib" "Generated from lib" replaced "${contents}")
    if(filename STREQUAL "EscriptParser.cpp")
      string(REPLACE "EscriptParser::EscriptParser(TokenStream *input)" "EscriptParser::EscriptParser(antlr4::TokenStream *input)" replaced "${replaced}")
    endif()
    file(WRITE ${file} "${replaced}")
  endforeach()
else()
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
    add_custom_command(TARGET escriptgrammar
      POST_BUILD COMMAND ${CMAKE_COMMAND} -Dpath=${POL_EXT_LIB_DIR}/EscriptGrammar -P ${CMAKE_CURRENT_LIST_FILE} 
      COMMENT "Fixing files"
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
endif()
