
// Generated from lib/EscriptGrammar/EscriptLexer.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"


namespace EscriptGrammar {


class  EscriptLexer : public antlr4::Lexer {
public:
  enum {
    IF = 1, THEN = 2, ELSEIF = 3, ENDIF = 4, ELSE = 5, GOTO = 6, RETURN = 7, 
    TOK_CONST = 8, VAR = 9, DO = 10, DOWHILE = 11, WHILE = 12, ENDWHILE = 13, 
    EXIT = 14, DECLARE = 15, FUNCTION = 16, ENDFUNCTION = 17, EXPORTED = 18, 
    USE = 19, INCLUDE = 20, BREAK = 21, CONTINUE = 22, FOR = 23, ENDFOR = 24, 
    TO = 25, FOREACH = 26, ENDFOREACH = 27, REPEAT = 28, UNTIL = 29, PROGRAM = 30, 
    ENDPROGRAM = 31, CASE = 32, DEFAULT = 33, ENDCASE = 34, ENUM = 35, ENDENUM = 36, 
    DOWNTO = 37, STEP = 38, REFERENCE = 39, TOK_OUT = 40, INOUT = 41, BYVAL = 42, 
    STRING = 43, TOK_LONG = 44, INTEGER = 45, UNSIGNED = 46, SIGNED = 47, 
    REAL = 48, FLOAT = 49, DOUBLE = 50, AS = 51, IS = 52, AND_A = 53, AND_B = 54, 
    OR_A = 55, OR_B = 56, BANG_A = 57, BANG_B = 58, BYREF = 59, UNUSED = 60, 
    TOK_ERROR = 61, HASH = 62, DICTIONARY = 63, STRUCT = 64, ARRAY = 65, 
    STACK = 66, TOK_IN = 67, DECIMAL_LITERAL = 68, HEX_LITERAL = 69, OCT_LITERAL = 70, 
    BINARY_LITERAL = 71, FLOAT_LITERAL = 72, HEX_FLOAT_LITERAL = 73, CHAR_LITERAL = 74, 
    STRING_LITERAL = 75, NULL_LITERAL = 76, LPAREN = 77, RPAREN = 78, LBRACK = 79, 
    RBRACK = 80, LBRACE = 81, RBRACE = 82, DOT = 83, ARROW = 84, MUL = 85, 
    DIV = 86, MOD = 87, ADD = 88, SUB = 89, ADD_ASSIGN = 90, SUB_ASSIGN = 91, 
    MUL_ASSIGN = 92, DIV_ASSIGN = 93, MOD_ASSIGN = 94, LE = 95, LT = 96, 
    GE = 97, GT = 98, RSHIFT = 99, LSHIFT = 100, BITAND = 101, CARET = 102, 
    BITOR = 103, NOTEQUAL_A = 104, NOTEQUAL_B = 105, EQUAL_DEPRECATED = 106, 
    EQUAL = 107, ASSIGN = 108, ADDMEMBER = 109, DELMEMBER = 110, CHKMEMBER = 111, 
    SEMI = 112, COMMA = 113, TILDE = 114, AT = 115, COLONCOLON = 116, COLON = 117, 
    INC = 118, DEC = 119, ELVIS = 120, WS = 121, COMMENT = 122, LINE_COMMENT = 123, 
    IDENTIFIER = 124
  };

  enum {
    COMMENTS = 2
  };

  EscriptLexer(antlr4::CharStream *input);
  ~EscriptLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace EscriptGrammar
