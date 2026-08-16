/** @file
 *
 * @par History
 */

#include <string>

#include "bscript/token.h"
#include "bscript/tokens.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
using namespace Bscript;

namespace
{
// The token's string is protected, so every case is built through setStr rather than by
// aggregate initialisation. Only the fields tostring() reads are set; the rest keep the
// constructor's values.
Token make( BTokenId id, int lval = 0, std::string str = {} )
{
  Token tok;
  tok.id = id;
  tok.lval = lval;
  tok.setStr( std::move( str ) );
  return tok;
}

Token make_typed( BTokenId id, BTokenType type, int lval = 0 )
{
  Token tok = make( id, lval );
  tok.type = type;
  return tok;
}

Token make_double( double dval )
{
  Token tok = make( TOK_DOUBLE );
  tok.dval = dval;
  return tok;
}

Token make_func( unsigned char module, int lval, std::string str )
{
  Token tok = make( TOK_FUNC, lval, std::move( str ) );
  tok.module = module;
  return tok;
}

// Formatting a token goes through the fmt formatter, which is the one thing in the file besides
// the switch itself.
#define T_TOK( tok, res ) UnitTest( []() { return fmt::format( "{}", tok ); }, res, "token " #tok )

void test_literals()
{
  T_TOK( make( TOK_LONG, 42 ), "42L" );
  T_TOK( make( TOK_LONG, -7 ), "-7L" );
  T_TOK( make_double( 1.5 ), "1.5LF" );
  T_TOK( make_double( -2.25 ), "-2.25LF" );
  T_TOK( make_double( 0.0 ), "0LF" );
  // any non-zero lval is the true arm
  T_TOK( make( TOK_BOOL, 1 ), "true (boolean)" );
  T_TOK( make( TOK_BOOL, 2 ), "true (boolean)" );
  T_TOK( make( TOK_BOOL, 0 ), "false (boolean)" );
  T_TOK( make( TOK_STRING, 0, "hello" ), "\"hello\"" );
  T_TOK( make( TOK_STRING, 0, "" ), "\"\"" );
  T_TOK( make( TOK_IDENT, 0, "myvar" ), "myvar" );
  T_TOK( make( TOK_ERROR ), "error" );
  T_TOK( make( TOK_DICTIONARY ), "dictionary" );
  T_TOK( make( TOK_ARRAY ), "array" );
  T_TOK( make( TOK_STRUCT ), "struct" );
  T_TOK( make( INS_UNINIT ), "uninit" );
  T_TOK( make( TOK_CLASSINST, 7 ), "class instance #7" );
  T_TOK( make( TOK_REGEXP ), "create-regular-expression" );
  T_TOK( make( TOK_INTERPOLATE_STRING, 3 ), "interpolate string (3 parts)" );
  T_TOK( make( TOK_FORMAT_EXPRESSION ), "format expression" );
  T_TOK( make( INS_DECLARE_ARRAY ), "declare array" );
  T_TOK( make( INS_DICTIONARY_ADDMEMBER ), "add dictionary member" );
}

void test_operators()
{
  T_TOK( make( TOK_ADD ), "+" );
  T_TOK( make( TOK_SUBTRACT ), "-" );
  T_TOK( make( TOK_MULT ), "*" );
  T_TOK( make( TOK_DIV ), "/" );
  T_TOK( make( TOK_MODULUS ), "%" );
  T_TOK( make( TOK_PLUSEQUAL ), "+=" );
  T_TOK( make( TOK_MINUSEQUAL ), "-=" );
  T_TOK( make( TOK_TIMESEQUAL ), "*=" );
  T_TOK( make( TOK_DIVIDEEQUAL ), "/=" );
  T_TOK( make( TOK_MODULUSEQUAL ), "%=" );
  T_TOK( make( TOK_INSERTINTO ), "init{}" );

  T_TOK( make( TOK_LESSTHAN ), "<" );
  T_TOK( make( TOK_LESSEQ ), "<=" );
  T_TOK( make( TOK_GRTHAN ), ">" );
  T_TOK( make( TOK_GREQ ), ">=" );
  T_TOK( make( TOK_EQUAL1 ), "=" );
  T_TOK( make( TOK_EQUAL ), "==" );
  T_TOK( make( TOK_NEQ ), "<>" );
  T_TOK( make( TOK_AND ), "&&" );
  T_TOK( make( TOK_OR ), "||" );
  T_TOK( make( TOK_IN ), "in" );

  T_TOK( make( TOK_BITAND ), "&" );
  T_TOK( make( TOK_BITOR ), "|" );
  T_TOK( make( TOK_BITXOR ), "^" );
  T_TOK( make( TOK_BITWISE_NOT ), "~" );
  T_TOK( make( TOK_BSLEFT ), "<<" );
  T_TOK( make( TOK_BSRIGHT ), ">>" );

  T_TOK( make( TOK_UNPLUS ), "unary +" );
  T_TOK( make( TOK_UNMINUS ), "unary -" );
  T_TOK( make( TOK_UNPLUSPLUS ), "unary ++" );
  T_TOK( make( TOK_UNMINUSMINUS ), "unary --" );
  T_TOK( make( TOK_UNPLUSPLUS_POST ), "unary ++ post" );
  T_TOK( make( TOK_UNMINUSMINUS_POST ), "unary -- post" );
  T_TOK( make( TOK_LOG_NOT ), "!" );

  T_TOK( make( TOK_CONSUMER ), "#" );
  T_TOK( make( TOK_REFTO ), "refto" );
  T_TOK( make( TOK_UNUSED ), "unused" );
  T_TOK( make( TOK_TERM ), "Terminator" );
  T_TOK( make( TOK_LPAREN ), "(" );
  T_TOK( make( TOK_RPAREN ), ")" );
  T_TOK( make( TOK_LBRACKET ), "[" );
  T_TOK( make( TOK_RBRACKET ), "]" );
  T_TOK( make( TOK_LBRACE ), "{" );
  T_TOK( make( TOK_RBRACE ), "}" );
  T_TOK( make( TOK_COMMA ), "','" );
  T_TOK( make( TOK_SEMICOLON ), "';'" );
  T_TOK( make( RSV_COLON ), "':'" );

  T_TOK( make( TOK_SPREAD, 1 ), "spread-into" );
  T_TOK( make( TOK_SPREAD, 0 ), "create-spread" );
  T_TOK( make( INS_LOGICAL_CONVERT ), "logical convert" );
}

void test_variables()
{
  // A named variable prints its number and its name; the name is optional and its absence leaves
  // the separating space behind.
  T_TOK( make( TOK_LOCALVAR, 3, "x" ), "local #3  (x)" );
  T_TOK( make( TOK_LOCALVAR, 3 ), "local #3 " );
  T_TOK( make( TOK_GLOBALVAR, 2, "g" ), "global #2  (g)" );
  T_TOK( make( TOK_GLOBALVAR, 2 ), "global #2 " );

  T_TOK( make( TOK_ASSIGN ), ":=" );
  T_TOK( make( INS_ASSIGN_CONSUME ), ":= #" );
  T_TOK( make( INS_ASSIGN_LOCALVAR, 1, "a" ), "local1  (a) :=" );
  T_TOK( make( INS_ASSIGN_LOCALVAR, 1 ), "local1  :=" );
  T_TOK( make( INS_ASSIGN_GLOBALVAR, 4, "b" ), "global4  (b) :=" );
  T_TOK( make( INS_ASSIGN_GLOBALVAR, 4 ), "global4  :=" );

  T_TOK( make( RSV_LOCAL, 2 ), "decl local #2" );
  T_TOK( make( RSV_GLOBAL, 2 ), "decl global #2" );
  T_TOK( make( INS_TAKE_LOCAL, 1 ), "take local #1" );
  T_TOK( make( INS_TAKE_GLOBAL, 1 ), "take global #1" );
  T_TOK( make( CTRL_MAKELOCAL ), "makelocal" );

  T_TOK( make( TOK_ARRAY_SUBSCRIPT, 1 ), "[] 1" );
  T_TOK( make( INS_SUBSCRIPT_ASSIGN, 2 ), "[] := (2)" );
  T_TOK( make( INS_SUBSCRIPT_ASSIGN_CONSUME, 2 ), "[] := (2) #" );
  // one comma per subscript beyond the first, so a single subscript prints nothing between the
  // brackets
  T_TOK( make( INS_MULTISUBSCRIPT, 3 ), "[,,]" );
  T_TOK( make( INS_MULTISUBSCRIPT, 1 ), "[]" );
  T_TOK( make( INS_MULTISUBSCRIPT_ASSIGN, 3 ), "[,,] :=" );
  T_TOK( make( INS_MULTISUBSCRIPT_ASSIGN, 1 ), "[] :=" );

  // the count is the low seven bits; a rest element sets the high bit and puts its index above it
  T_TOK( make( INS_UNPACK_SEQUENCE, 3 ), "unpack sequence (3 elements, rest index -1)" );
  T_TOK( make( INS_UNPACK_SEQUENCE, 0x83 ), "unpack sequence (3 elements, rest index 1)" );
  T_TOK( make( INS_UNPACK_INDICES, 2 ), "unpack indices (2 elements, rest index -1)" );
  T_TOK( make( INS_UNPACK_INDICES, 0x102 ), "unpack indices (2 elements, rest index 2)" );
}

void test_members()
{
  T_TOK( make( TOK_MEMBER ), "." );
  T_TOK( make( TOK_ADDMEMBER ), ".+" );
  T_TOK( make( TOK_DELMEMBER ), ".-" );
  T_TOK( make( TOK_CHKMEMBER ), ".?" );
  T_TOK( make( INS_ADDMEMBER2, 0, "m" ), "addmember(m)" );
  T_TOK( make( INS_ADDMEMBER_ASSIGN, 0, "m" ), "addmember-assign(m)" );

  T_TOK( make( INS_GET_MEMBER, 0, "name" ), "get member 'name'" );
  T_TOK( make( INS_SET_MEMBER, 0, "name" ), "set member 'name'" );
  T_TOK( make( INS_SET_MEMBER_CONSUME, 0, "name" ), "set member 'name' #" );

  // by id, the name comes from the member table: 0 is "x", 3 is "name"
  T_TOK( make( INS_GET_MEMBER_ID, 0 ), "get member id 'x' (0)" );
  T_TOK( make( INS_GET_MEMBER_ID, 3 ), "get member id 'name' (3)" );
  T_TOK( make( INS_SET_MEMBER_ID, 3 ), "set member id 'name' (3)" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME, 3 ), "set member id 'name' (3) #" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL, 0 ), "set member id 'x' (0)  += #" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL, 0 ), "set member id 'x' (0)  -= #" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL, 0 ), "set member id 'x' (0)  *= #" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL, 0 ), "set member id 'x' (0)  /= #" );
  T_TOK( make( INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL, 0 ), "set member id 'x' (0)  %= #" );
  T_TOK( make( INS_SET_MEMBER_ID_UNPLUSPLUS, 1 ), "set member id 'y' unary ++" );
  T_TOK( make( INS_SET_MEMBER_ID_UNMINUSMINUS, 1 ), "set member id 'y' unary --" );
  T_TOK( make( INS_SET_MEMBER_ID_UNPLUSPLUS_POST, 1 ), "set member id 'y' unary ++ post" );
  T_TOK( make( INS_SET_MEMBER_ID_UNMINUSMINUS_POST, 1 ), "set member id 'y' unary -- post" );
}

void test_calls()
{
  T_TOK( make( INS_CALL_METHOD, 2, "foo" ), "Call Method foo (2 params) " );
  // by id the name comes from the method table, where 0 is "isa", and the parameter count is
  // carried in the token's type rather than in lval
  T_TOK( make_typed( INS_CALL_METHOD_ID, static_cast<BTokenType>( 2 ), 0 ),
         "Call Method id isa (#0, 2 params)" );
  T_TOK( make_typed( INS_CALL_METHOD_ID, static_cast<BTokenType>( 0 ), 1 ),
         "Call Method id set_member (#1, 0 params)" );

  T_TOK( make( TOK_USERFUNC, 0, "f" ), "User Function f" );
  T_TOK( make( TOK_USERFUNC ), "User Function --function name not available--" );
  T_TOK( make( TOK_FUNCREF, 3, "f" ), "Function Ref f@3" );
  T_TOK( make( TOK_FUNCREF, 3 ), "Function Ref --function name not available--@3" );
  T_TOK( make_func( 2, 5, "Print" ), "Func(2,5): Print" );
  T_TOK( make_func( 0, 0, "" ), "Func(0,0): <unknown>" );

  T_TOK( make( CTRL_JSR_USERFUNC, 9 ), "jmp userfunc @9" );
  T_TOK( make( INS_POP_PARAM, 0, "p" ), "pop param 'p'" );
  T_TOK( make( INS_POP_PARAM_BYREF, 0, "p" ), "pop param byref 'p'" );
  T_TOK( make( INS_GET_ARG, 0, "p" ), "get arg 'p'" );
  T_TOK( make( INS_CHECK_MRO, 2 ), "check mro (this at offset 2)" );
}

void test_control_flow()
{
  T_TOK( make( RSV_JMPIFTRUE, 12 ), "if true goto 12" );
  T_TOK( make( RSV_JMPIFFALSE, 12 ), "if false goto 12" );
  T_TOK( make( RSV_GOTO, 4 ), "goto 4" );
  // no space after the keyword, unlike goto
  T_TOK( make( RSV_GOSUB, 4 ), "gosub4" );
  T_TOK( make( RSV_EXIT ), "exit" );
  T_TOK( make( RSV_RETURN ), "return" );
  T_TOK( make( INS_CASEJMP ), "casejmp" );
  T_TOK( make( CTRL_LEAVE_BLOCK, 3 ), "leave block(3)" );
  T_TOK( make( INS_SKIPIFTRUE_ELSE_CONSUME, 4 ),
         "peek at top of stack; skip 4 instructions if true, otherwise consume it" );
  // the jump direction is read from the type, not from lval
  T_TOK( make_typed( INS_LOGICAL_JUMP, TYP_LOGICAL_JUMP_FALSE, 8 ), "logical jump if false to 8" );
  T_TOK( make_typed( INS_LOGICAL_JUMP, TYP_OPERAND, 8 ), "logical jump if true to 8" );

  T_TOK( make( INS_INITFOR, 5 ), "initfor @5" );
  T_TOK( make( INS_NEXTFOR, 5 ), "nextfor @5" );
  T_TOK( make( INS_INITFOREACH, 10 ), "initforeach @10" );
  T_TOK( make( INS_STEPFOREACH, 10 ), "stepforeach @10" );

  T_TOK( make( CTRL_LABEL, 0, "lbl" ), "lbl:" );
  T_TOK( make( CTRL_PROGEND ), "progend" );
  T_TOK( make( CTRL_STATEMENTBEGIN, 0, "a := 1;" ), "[a := 1;]" );
  T_TOK( make( CTRL_STATEMENTBEGIN ), "[--source not available--]" );
}

void test_reserved_words()
{
  T_TOK( make( RSV_ST_IF ), "if" );
  T_TOK( make( RSV_VAR ), "var" );
  T_TOK( make( RSV_CONST ), "const" );
  T_TOK( make( RSV_FUNCTION ), "function" );
  T_TOK( make( RSV_ENDFUNCTION ), "endfunction" );
  T_TOK( make( RSV_DO ), "do" );
  T_TOK( make( RSV_DOWHILE ), "dowhile" );
  T_TOK( make( RSV_WHILE ), "while" );
  T_TOK( make( RSV_ENDWHILE ), "endwhile" );
  T_TOK( make( RSV_REPEAT ), "repeat" );
  T_TOK( make( RSV_UNTIL ), "until" );
  T_TOK( make( RSV_FOR ), "for" );
  T_TOK( make( RSV_ENDFOR ), "endfor" );
  T_TOK( make( RSV_FOREACH ), "foreach" );
  T_TOK( make( RSV_ENDFOREACH ), "endforeach" );
  T_TOK( make( RSV_PROGRAM ), "program" );
  T_TOK( make( RSV_ENDPROGRAM ), "endprogram" );
  T_TOK( make( RSV_ENUM ), "enum" );
  T_TOK( make( RSV_ENDENUM ), "endenum" );
  T_TOK( make( RSV_ELVIS ), "?: (elvis)" );
  T_TOK( make( RSV_USE_MODULE ), "use module" );
  T_TOK( make( RSV_INCLUDE_FILE ), "include file" );
}

void test_unknown_tokens()
{
  // Tokens the switch does not name print their raw numbers, and their string only when they
  // carry one.
  T_TOK( make( TOK_STACK ), "Unknown Token: (62,0)" );
  T_TOK( make( TOK_STACK, 0, "z" ), "Unknown Token: (62,0,'z')" );
  T_TOK( make_typed( TOK_FUNCTOR, TYP_OPERAND ), "Unknown Token: (110,1)" );
  T_TOK( make( TOK_CONSTRUCTINST ), "Unknown Token: (114,0)" );
  T_TOK( make( TOK_IS ), "Unknown Token: (116,0)" );
}
#undef T_TOK
}  // namespace

void tokens_test()
{
  test_literals();
  test_operators();
  test_variables();
  test_members();
  test_calls();
  test_control_flow();
  test_reserved_words();
  test_unknown_tokens();
}

}  // namespace Pol::Testing
