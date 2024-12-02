
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
    INTERPOLATED_STRING_START = 80, LPAREN = 81, RPAREN = 82, LBRACK = 83, 
    RBRACK = 84, LBRACE = 85, RBRACE = 86, DOT = 87, ARROW = 88, MUL = 89, 
    DIV = 90, MOD = 91, ADD = 92, SUB = 93, ADD_ASSIGN = 94, SUB_ASSIGN = 95, 
    MUL_ASSIGN = 96, DIV_ASSIGN = 97, MOD_ASSIGN = 98, LE = 99, LT = 100, 
    GE = 101, GT = 102, LSHIFT = 103, BITAND = 104, CARET = 105, BITOR = 106, 
    NOTEQUAL_A = 107, NOTEQUAL_B = 108, EQUAL_DEPRECATED = 109, EQUAL = 110, 
    ASSIGN = 111, ADDMEMBER = 112, DELMEMBER = 113, CHKMEMBER = 114, SEMI = 115, 
    COMMA = 116, TILDE = 117, AT = 118, COLONCOLON = 119, COLON = 120, INC = 121, 
    DEC = 122, ELVIS = 123, QUESTION = 124, WS = 125, COMMENT = 126, LINE_COMMENT = 127, 
    IDENTIFIER = 128, DOUBLE_LBRACE_INSIDE = 129, LBRACE_INSIDE = 130, REGULAR_CHAR_INSIDE = 131, 
    DOUBLE_QUOTE_INSIDE = 132, DOUBLE_RBRACE = 133, STRING_LITERAL_INSIDE = 134, 
    CLOSE_RBRACE_INSIDE = 135, FORMAT_STRING = 136
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
