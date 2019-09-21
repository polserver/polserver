/** @file
 *
 * @par History
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: ParseErrorStr changed little bit
 */


#ifndef __PARSER_H
#define __PARSER_H

#include <iosfwd>
#include <stddef.h>
#include <string>

#include "tokens.h"

namespace Pol
{
namespace Bscript
{
class ModuleFunction;
class Token;
class UserFunction;
}  // namespace Bscript
}  // namespace Pol

#ifndef __TOKEN_H
#include "token.h"
#endif
#ifndef __OPERATOR_H
#include "operator.h"
#endif

#include <queue>
#include <stack>
#include <vector>

namespace Pol
{
namespace Bscript
{
class CompilerContext;

typedef enum
{
  PERR_NONE,
  PERR_UNEXRPAREN,  // unexpected RIGHT Paren
  PERR_MISSLPAREN,
  PERR_MISSRPAREN,    // missing RPAREN
  PERR_BADTOKEN,      // bad token ??
  PERR_BADOPER,       // unknown operator..
  PERR_WAAH,          // god knows what happened
  PERR_UNTERMSTRING,  // "abcd  (not terminated with '"')
  PERR_INVESCAPE,     // an invalid escape sequence (eg. \xFG)
  PERR_TOOFEWARGS,
  PERR_TOOMANYARGS,
  PERR_UNEXPCOMMA,
  PERR_ILLEGALCONS,  // illegal construction
  PERR_MISSINGDELIM,
  PERR_NOTLEGALHERE,
  PERR_PROCNOTALLOWED,
  PERR_UNEXPSEMI,
  PERR_EXPWHILE,
  PERR_UNEXRBRACKET,
  PERR_MISSRBRACKET,
  PERR_NUM_ERRORS
} ParseError;

extern const char* ParseErrorStr[];


class Expression
{
public:
  ~Expression();

  void eat( Expression& expr );
  void eat2( Expression& expr );
  void optimize();
  void optimize_binary_operations();
  void optimize_unary_operations();
  void optimize_assignments();
  int get_num_tokens( int idx ) const;
  bool optimize_token( int i );

  typedef std::vector<Token*> Tokens;
  Tokens tokens;

public:
  std::stack<Token*> TX;
  std::queue<Token*> CA;
};


const unsigned EXPR_FLAG_SEMICOLON_TERM_ALLOWED = 0x0001;
const unsigned EXPR_FLAG_COMMA_TERM_ALLOWED = 0x0002;
const unsigned EXPR_FLAG_RIGHTPAREN_TERM_ALLOWED = 0x0004;
const unsigned EXPR_FLAG_SINGLE_ELEMENT = 0x0008;
const unsigned EXPR_FLAG_ENDENUM_TERM_ALLOWED = 0x0010;
const unsigned EXPR_FLAG_RIGHTBRACE_TERM_ALLOWED = 0x0020;
const unsigned EXPR_FLAG_TO_TERM_ALLOWED = 0x0040;
const unsigned EXPR_FLAG_AUTO_TERM_ALLOWED = 0x0080;
const unsigned EXPR_FLAG_CONSUME_RESULT = 0x0100;
const unsigned EXPR_FLAG_DICTKEY_TERM_ALLOWED = 0x0200;


class Parser
{
public:
  Parser();
  virtual ~Parser() {}
  Parser& operator=( const Parser& ) { return *this; }
  int quiet;
  ParseError err;

  char ext_err[50];
  char buffer[51];

  bool contains_tabs;

public:
public:
  void reinit( Expression& ex );

  static void write_words( std::ostream& os );
  virtual int recognize_binary( Token& tok, const char* buf, const char** s );
  virtual int recognize_unary( Token& tok, const char* buf );
  virtual int recognize( Token& tok, const char* buf, const char** s );
  virtual bool recognize_reserved_word( Token& tok, const char* buf );

  virtual int tryOperator( Token& tok, const char* buf, const char** s, Operator* opList, int n_ops,
                           char* opbuf );
  virtual int tryBinaryOperator( Token& tok, CompilerContext& ctx );
  virtual int tryUnaryOperator( Token& tok, CompilerContext& ctx );

  virtual int tryNumeric( Token& tok, CompilerContext& ctx );
  virtual int tryLiteral( Token& tok, CompilerContext& ctx );

  virtual int peekToken( const CompilerContext& ctx, Token& token, Expression* expr = nullptr );
  virtual int getToken( CompilerContext& ctx, Token& token, Expression* expr = nullptr );


  virtual int parseToken( CompilerContext& ctx, Expression& expr, Token* token ) = 0;
  int IP( Expression& expr, char* s );

  void setQuiet( int x ) { quiet = x; }
};


class SmartParser : public Parser
{
public:
  virtual ~SmartParser() {}

protected:
  virtual int tryLiteral( Token& tok, CompilerContext& ctx ) override;

  class ModuleFunction* modfunc_;
  UserFunction* userfunc_;

public:
  SmartParser() : Parser(), modfunc_( nullptr ), userfunc_( nullptr ) {}
  SmartParser& operator=( const SmartParser& ) { return *this; }
  virtual int isLegal( Token& tok );
  virtual int isOkay( const Token& token, const Token& last_token );

  virtual int isFunc( Token& tok, ModuleFunction** v ) = 0;

  virtual int isUserFunc( Token& tok, UserFunction** userfunc );

  virtual int parseToken( CompilerContext& ctx, Expression& expr, Token* ) override;
  virtual int getToken( CompilerContext& ctx, Token& token, Expression* expr = nullptr ) override;

  bool callingMethod( CompilerContext& ctx );

  int getArgs( Expression& expr, CompilerContext& ctx );
  virtual int getUserArgs( Expression& expr, CompilerContext& ctx, bool inject_jsr = true ) = 0;
  virtual int getArrayElements( Expression& expr, CompilerContext& ctx ) = 0;
  virtual int getNewArrayElements( Expression& expr, CompilerContext& ctx ) = 0;
  virtual int getStructMembers( Expression& expr, CompilerContext& ctx ) = 0;
  virtual int getDictionaryMembers( Expression& expr, CompilerContext& ctx ) = 0;
  virtual int getMethodArguments( Expression& expr, CompilerContext& ctx, int& nargs ) = 0;
  virtual int getFunctionPArgument( Expression& expr, CompilerContext& ctx, Token* tok ) = 0;

  int IIP( Expression& expr, CompilerContext& ctx, unsigned expr_flags );
  int IP( Expression& expr, char* s );
  int IP( Expression& expr, CompilerContext& ctx );
};

inline int SmartParser::isLegal( Token& )
{
  return 1;
}
inline int SmartParser::isUserFunc( Token&, UserFunction** )
{
  return 0;
}
}  // namespace Bscript
}  // namespace Pol
#endif
