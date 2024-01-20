
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
    REAL = 47, FLOAT = 48, DOUBLE = 49, AS = 50, IS = 51, AND_A = 52, AND_B = 53,
    OR_A = 54, OR_B = 55, BANG_A = 56, BANG_B = 57, BYREF = 58, UNUSED = 59,
    TOK_ERROR = 60, HASH = 61, DICTIONARY = 62, STRUCT = 63, ARRAY = 64,
    STACK = 65, TOK_IN = 66, UNINIT = 67, BOOL_TRUE = 68, BOOL_FALSE = 69,
    DECIMAL_LITERAL = 70, HEX_LITERAL = 71, OCT_LITERAL = 72, BINARY_LITERAL = 73,
    FLOAT_LITERAL = 74, HEX_FLOAT_LITERAL = 75, STRING_LITERAL = 76, INTERPOLATED_STRING_START = 77,
    LPAREN = 78, RPAREN = 79, LBRACK = 80, RBRACK = 81, LBRACE = 82, RBRACE = 83,
    DOT = 84, ARROW = 85, MUL = 86, DIV = 87, MOD = 88, ADD = 89, SUB = 90,
    ADD_ASSIGN = 91, SUB_ASSIGN = 92, MUL_ASSIGN = 93, DIV_ASSIGN = 94,
    MOD_ASSIGN = 95, LE = 96, LT = 97, GE = 98, GT = 99, RSHIFT = 100, LSHIFT = 101,
    BITAND = 102, CARET = 103, BITOR = 104, NOTEQUAL_A = 105, NOTEQUAL_B = 106,
    EQUAL_DEPRECATED = 107, EQUAL = 108, ASSIGN = 109, ADDMEMBER = 110,
    DELMEMBER = 111, CHKMEMBER = 112, SEMI = 113, COMMA = 114, TILDE = 115,
    AT = 116, COLONCOLON = 117, COLON = 118, INC = 119, DEC = 120, ELVIS = 121,
    QUESTION = 122, WS = 123, COMMENT = 124, LINE_COMMENT = 125, IDENTIFIER = 126,
    DOUBLE_LBRACE_INSIDE = 127, LBRACE_INSIDE = 128, REGULAR_CHAR_INSIDE = 129,
    DOUBLE_QUOTE_INSIDE = 130, DOUBLE_RBRACE = 131, STRING_LITERAL_INSIDE = 132,
    CLOSE_RBRACE_INSIDE = 133, FORMAT_STRING = 134
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
