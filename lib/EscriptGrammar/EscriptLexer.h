
// Generated from lib/EscriptGrammar/EscriptLexer.g4 by ANTLR 4.8

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
    STACK = 65, TOK_IN = 66, DECIMAL_LITERAL = 67, HEX_LITERAL = 68, OCT_LITERAL = 69, 
    BINARY_LITERAL = 70, FLOAT_LITERAL = 71, HEX_FLOAT_LITERAL = 72, STRING_LITERAL = 73, 
    LPAREN = 74, RPAREN = 75, LBRACK = 76, RBRACK = 77, LBRACE = 78, RBRACE = 79, 
    DOT = 80, ARROW = 81, MUL = 82, DIV = 83, MOD = 84, ADD = 85, SUB = 86, 
    ADD_ASSIGN = 87, SUB_ASSIGN = 88, MUL_ASSIGN = 89, DIV_ASSIGN = 90, 
    MOD_ASSIGN = 91, LE = 92, LT = 93, GE = 94, GT = 95, RSHIFT = 96, LSHIFT = 97, 
    BITAND = 98, CARET = 99, BITOR = 100, NOTEQUAL_A = 101, NOTEQUAL_B = 102, 
    EQUAL_DEPRECATED = 103, EQUAL = 104, ASSIGN = 105, ADDMEMBER = 106, 
    DELMEMBER = 107, CHKMEMBER = 108, SEMI = 109, COMMA = 110, TILDE = 111, 
    AT = 112, COLONCOLON = 113, COLON = 114, INC = 115, DEC = 116, ELVIS = 117, 
    WS = 118, COMMENT = 119, LINE_COMMENT = 120, IDENTIFIER = 121
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
