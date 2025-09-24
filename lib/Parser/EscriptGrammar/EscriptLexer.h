
// Generated from EscriptLexer.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"


namespace EscriptGrammar {


class  EscriptLexer : public antlr4::Lexer {
public:
  enum {
    IF = 1, THEN = 2, ELSEIF = 3, ENDIF = 4, ELSE = 5, GOTO = 6, RETURN = 7, 
    TOK_CONST = 8, VAR = 9, DO = 10, DOWHILE = 11, WHILE = 12, ENDWHILE = 13, 
    EXIT = 14, FUNCTION = 15, ENDFUNCTION = 16, EXPORTED = 17, USE = 18, 
    INCLUDE = 19, BREAK = 20, CONTINUE = 21, FOR = 22, ENDFOR = 23, TO = 24, 
    FOREACH = 25, ENDFOREACH = 26, REPEAT = 27, UNTIL = 28, PROGRAM = 29, 
    ENDPROGRAM = 30, CASE = 31, DEFAULT = 32, ENDCASE = 33, ENUM = 34, ENDENUM = 35, 
    CLASS = 36, ENDCLASS = 37, DOWNTO = 38, STEP = 39, REFERENCE = 40, TOK_OUT = 41, 
    INOUT = 42, BYVAL = 43, STRING = 44, TOK_LONG = 45, INTEGER = 46, UNSIGNED = 47, 
    SIGNED = 48, REAL = 49, FLOAT = 50, DOUBLE = 51, AS = 52, ELLIPSIS = 53, 
    AND_A = 54, AND_B = 55, OR_A = 56, OR_B = 57, BANG_A = 58, BANG_B = 59, 
    BYREF = 60, UNUSED = 61, TOK_ERROR = 62, HASH = 63, DICTIONARY = 64, 
    STRUCT = 65, ARRAY = 66, STACK = 67, TOK_IN = 68, UNINIT = 69, BOOL_TRUE = 70, 
    BOOL_FALSE = 71, IS = 72, DECIMAL_LITERAL = 73, HEX_LITERAL = 74, OCT_LITERAL = 75, 
    BINARY_LITERAL = 76, FLOAT_LITERAL = 77, HEX_FLOAT_LITERAL = 78, STRING_LITERAL = 79, 
    REGEXP_LITERAL = 80, INTERPOLATED_STRING_START = 81, LPAREN = 82, RPAREN = 83, 
    LBRACK = 84, RBRACK = 85, LBRACE = 86, RBRACE = 87, DOT = 88, ARROW = 89, 
    MUL = 90, DIV = 91, MOD = 92, ADD = 93, SUB = 94, ADD_ASSIGN = 95, SUB_ASSIGN = 96, 
    MUL_ASSIGN = 97, DIV_ASSIGN = 98, MOD_ASSIGN = 99, LE = 100, LT = 101, 
    GE = 102, GT = 103, RSHIFT = 104, LSHIFT = 105, BITAND = 106, CARET = 107, 
    BITOR = 108, NOTEQUAL_A = 109, NOTEQUAL_B = 110, EQUAL_DEPRECATED = 111, 
    EQUAL = 112, ASSIGN = 113, ADDMEMBER = 114, DELMEMBER = 115, CHKMEMBER = 116, 
    SEMI = 117, COMMA = 118, TILDE = 119, AT = 120, COLONCOLON = 121, COLON = 122, 
    INC = 123, DEC = 124, ELVIS = 125, QUESTION = 126, WS = 127, COMMENT = 128, 
    LINE_COMMENT = 129, IDENTIFIER = 130, DOUBLE_LBRACE_INSIDE = 131, LBRACE_INSIDE = 132, 
    REGULAR_CHAR_INSIDE = 133, DOUBLE_QUOTE_INSIDE = 134, DOUBLE_RBRACE = 135, 
    STRING_LITERAL_INSIDE = 136, CLOSE_RBRACE_INSIDE = 137, FORMAT_STRING = 138
  };

  enum {
    COMMENTS = 2
  };

  enum {
    INTERPOLATION_STRING = 1, INTERPOLATION_FORMAT = 2
  };

  explicit EscriptLexer(antlr4::CharStream *input);

  ~EscriptLexer() override;


      int interpolatedStringLevel = 0;
      std::stack<int> curlyLevels;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.
  void INTERPOLATED_STRING_STARTAction(antlr4::RuleContext *context, size_t actionIndex);
  void LBRACEAction(antlr4::RuleContext *context, size_t actionIndex);
  void RBRACEAction(antlr4::RuleContext *context, size_t actionIndex);
  void COLONAction(antlr4::RuleContext *context, size_t actionIndex);
  void LBRACE_INSIDEAction(antlr4::RuleContext *context, size_t actionIndex);
  void DOUBLE_QUOTE_INSIDEAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_RBRACE_INSIDEAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.

};

}  // namespace EscriptGrammar
