
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
    DOWNTO = 36, STEP = 37, REFERENCE = 38, TOK_OUT = 39, INOUT = 40, BYVAL = 41,
    STRING = 42, TOK_LONG = 43, INTEGER = 44, UNSIGNED = 45, SIGNED = 46,
    REAL = 47, FLOAT = 48, DOUBLE = 49, AS = 50, IS = 51, ELLIPSIS = 52,
    AND_A = 53, AND_B = 54, OR_A = 55, OR_B = 56, BANG_A = 57, BANG_B = 58,
    BYREF = 59, UNUSED = 60, TOK_ERROR = 61, HASH = 62, DICTIONARY = 63,
    STRUCT = 64, ARRAY = 65, STACK = 66, TOK_IN = 67, UNINIT = 68, BOOL_TRUE = 69,
    BOOL_FALSE = 70, DECIMAL_LITERAL = 71, HEX_LITERAL = 72, OCT_LITERAL = 73,
    BINARY_LITERAL = 74, FLOAT_LITERAL = 75, HEX_FLOAT_LITERAL = 76, STRING_LITERAL = 77,
    INTERPOLATED_STRING_START = 78, LPAREN = 79, RPAREN = 80, LBRACK = 81,
    RBRACK = 82, LBRACE = 83, RBRACE = 84, DOT = 85, ARROW = 86, MUL = 87,
    DIV = 88, MOD = 89, ADD = 90, SUB = 91, ADD_ASSIGN = 92, SUB_ASSIGN = 93,
    MUL_ASSIGN = 94, DIV_ASSIGN = 95, MOD_ASSIGN = 96, LE = 97, LT = 98,
    GE = 99, GT = 100, RSHIFT = 101, LSHIFT = 102, BITAND = 103, CARET = 104,
    BITOR = 105, NOTEQUAL_A = 106, NOTEQUAL_B = 107, EQUAL_DEPRECATED = 108,
    EQUAL = 109, ASSIGN = 110, ADDMEMBER = 111, DELMEMBER = 112, CHKMEMBER = 113,
    SEMI = 114, COMMA = 115, TILDE = 116, AT = 117, COLONCOLON = 118, COLON = 119,
    INC = 120, DEC = 121, ELVIS = 122, QUESTION = 123, WS = 124, COMMENT = 125,
    LINE_COMMENT = 126, IDENTIFIER = 127, DOUBLE_LBRACE_INSIDE = 128, LBRACE_INSIDE = 129,
    REGULAR_CHAR_INSIDE = 130, DOUBLE_QUOTE_INSIDE = 131, DOUBLE_RBRACE = 132,
    STRING_LITERAL_INSIDE = 133, CLOSE_RBRACE_INSIDE = 134, FORMAT_STRING = 135
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
