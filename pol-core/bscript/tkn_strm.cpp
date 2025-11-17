/** @file
 *
 * @par History
 */


#include <sstream>
#include <stddef.h>

#include "objmembers.h"
#include "objmethods.h"
#include "token.h"
#include "tokens.h"


namespace Pol
{
namespace Bscript
{
void Token::printOn( std::ostream& os ) const
{
  switch ( id )
  {
  case TOK_LONG:
    os << lval << "L";
    break;
  case TOK_DOUBLE:
    os << dval << "LF";
    break;
  case TOK_BOOL:
    os << fmt::format( "{} (boolean)", static_cast<bool>( lval ) );
    break;
  case TOK_IDENT:
    os << token;
    break;
  case INS_ADDMEMBER2:
    os << "addmember(" << token << ")";
    break;
  case INS_ADDMEMBER_ASSIGN:
    os << "addmember-assign(" << token << ")";
    break;
  case TOK_STRING:
    os << '\"' << token << '\"';
    break;
  case TOK_LOCALVAR:
    os << "local #" << lval;
    if ( !token.empty() )
      os << " (" << token << ")";
    break;
  case TOK_GLOBALVAR:
    os << "global #" << lval;
    if ( !token.empty() )
      os << " (" << token << ")";
    break;

  case TOK_ERROR:
    os << "error";
    break;
  case TOK_DICTIONARY:
    os << "dictionary";
    break;

  case TOK_MULT:
    os << "*";
    break;
  case TOK_DIV:
    os << "/";
    break;
  case TOK_ADD:
    os << "+";
    break;

  case TOK_INSERTINTO:
    os << "init{}";
    break;
  case TOK_SUBTRACT:
    os << "-";
    break;
  case TOK_PLUSEQUAL:
    os << "+=";
    break;
  case TOK_MINUSEQUAL:
    os << "-=";
    break;
  case TOK_TIMESEQUAL:
    os << "*=";
    break;
  case TOK_DIVIDEEQUAL:
    os << "/=";
    break;
  case TOK_MODULUSEQUAL:
    os << "%=";
    break;

  case TOK_ASSIGN:
    os << ":=";
    break;
  case INS_ASSIGN_LOCALVAR:
    os << "local" << lval;
    if ( !token.empty() )
      os << " (" << token << ")";
    os << " := ";
    break;
  case INS_ASSIGN_GLOBALVAR:
    os << "global" << lval;
    if ( !token.empty() )
      os << " (" << token << ")";
    os << " := ";
    break;
  case INS_ASSIGN_CONSUME:
    os << ":= #";
    break;
  case INS_SUBSCRIPT_ASSIGN_CONSUME:
    os << "[] := (" << lval << ") #";
    break;
  case INS_SUBSCRIPT_ASSIGN:
    os << "[] := (" << lval << ")";
    break;
  case TOK_LESSTHAN:
    os << "<";
    break;
  case TOK_LESSEQ:
    os << "<=";
    break;
  case TOK_GRTHAN:
    os << ">";
    break;
  case TOK_GREQ:
    os << ">=";
    break;
  case TOK_EQUAL1:
    os << "=";
    break;
  case TOK_EQUAL:
    os << "==";
    break;
  case TOK_NEQ:
    os << "<>";
    break;
  case TOK_AND:
    os << "&&";
    break;
  case TOK_OR:
    os << "||";
    break;
  case TOK_ARRAY_SUBSCRIPT:
    os << "[] " << lval;
    break;
  case INS_MULTISUBSCRIPT:
    os << "[";
    for ( int i = 1; i < lval; ++i )
      os << ",";
    os << "]";
    break;
  case INS_MULTISUBSCRIPT_ASSIGN:
    os << "[";
    for ( int i = 1; i < lval; ++i )
      os << ",";
    os << "] :=";
    break;
  case TOK_ADDMEMBER:
    os << ".+";
    break;
  case TOK_DELMEMBER:
    os << ".-";
    break;
  case TOK_CHKMEMBER:
    os << ".?";
    break;
  case TOK_MEMBER:
    os << ".";
    break;
  case INS_GET_MEMBER:
    os << "get member '" << token << "'";
    break;
  case INS_SET_MEMBER:
    os << "set member '" << token << "'";
    break;
  case INS_SET_MEMBER_CONSUME:
    os << "set member '" << token << "' #";
    break;
  case INS_GET_MEMBER_ID:
    os << "get member id '" << getObjMember( lval )->code << "' (" << lval << ")";
    break;
  case INS_SET_MEMBER_ID:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")";
    break;
  case INS_SET_MEMBER_ID_CONSUME:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ") #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")  += #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")  -= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")  *= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")  /= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
    os << "set member id '" << getObjMember( lval )->code << "' (" << lval << ")  %= #";
    break;

  case INS_CALL_METHOD_ID:
    os << "Call Method id " << getObjMethod( lval )->code << " (#" << lval << ", "
       << static_cast<int>( type ) << " params)";
    break;
  case TOK_IN:
    os << "in";
    break;
  case INS_DICTIONARY_ADDMEMBER:
    os << "add dictionary member";
    break;

  case TOK_UNPLUS:
    os << "unary +";
    break;
  case TOK_UNMINUS:
    os << "unary -";
    break;
  case TOK_LOG_NOT:
    os << "!";
    break;
  case TOK_CONSUMER:
    os << "#";
    break;
  case TOK_REFTO:
    os << "refto";
    break;
  case TOK_UNUSED:
    os << "unused";
    break;
  case TOK_BITAND:
    os << "&";
    break;
  case TOK_BITOR:
    os << "|";
    break;
  case TOK_BSRIGHT:
    os << ">>";
    break;
  case TOK_BSLEFT:
    os << "<<";
    break;
  case TOK_BITXOR:
    os << "^";
    break;
  case TOK_BITWISE_NOT:
    os << "~";
    break;
  case TOK_MODULUS:
    os << "%";
    break;

  case INS_INITFOREACH:
    os << "initforeach @" << lval;
    break;
  case INS_STEPFOREACH:
    os << "stepforeach @" << lval;
    break;
  case INS_CASEJMP:
    os << "casejmp";
    break;
  case INS_INITFOR:
    os << "initfor @" << lval;
    break;
  case INS_NEXTFOR:
    os << "nextfor @" << lval;
    break;

  case TOK_TERM:
    os << "Terminator";
    break;

  case TOK_LPAREN:
    os << "(";
    break;
  case TOK_RPAREN:
    os << ")";
    break;
  case TOK_LBRACKET:
    os << "[";
    break;
  case TOK_RBRACKET:
    os << "]";
    break;
  case TOK_LBRACE:
    os << "{";
    break;
  case TOK_RBRACE:
    os << "}";
    break;

  case RSV_JMPIFTRUE:
    os << "if true goto " << lval;
    break;
  case RSV_JMPIFFALSE:
    os << "if false goto " << lval;

    break;
  case RSV_ST_IF:
    os << "if";
    break;
  case RSV_GOTO:
    os << "goto " << lval;
    break;
  case RSV_GOSUB:
    os << "gosub" << lval;
    break;
  case RSV_EXIT:
    os << "exit";
    break;
  case RSV_RETURN:
    os << "return";
    break;
  case RSV_LOCAL:
    os << "decl local #" << lval;
    break;
  case RSV_GLOBAL:
    os << "decl global #" << lval;
    break;
  case RSV_VAR:
    os << "var";
    break;
  case RSV_CONST:
    os << "const";
    break;
  case RSV_FUNCTION:
    os << "function";
    break;
  case RSV_ENDFUNCTION:
    os << "endfunction";
    break;
  case RSV_DO:
    os << "do";
    break;
  case RSV_DOWHILE:
    os << "dowhile";
    break;
  case RSV_WHILE:
    os << "while";
    break;
  case RSV_ENDWHILE:
    os << "endwhile";
    break;
  case RSV_REPEAT:
    os << "repeat";
    break;
  case RSV_UNTIL:
    os << "until";
    break;
  case RSV_FOR:
    os << "for";
    break;
  case RSV_ENDFOR:
    os << "endfor";
    break;
  case RSV_FOREACH:
    os << "foreach";
    break;
  case RSV_ENDFOREACH:
    os << "endforeach";
    break;
  case INS_DECLARE_ARRAY:
    os << "declare array";
    break;
  case TOK_ARRAY:
    os << "array";
    break;
  case TOK_STRUCT:
    os << "struct";
    break;
  case TOK_CLASSINST:
    os << "class instance #" << lval;
    break;
  case INS_UNINIT:
    os << "uninit";
    break;
  case RSV_USE_MODULE:
    os << "use module";
    break;
  case RSV_INCLUDE_FILE:
    os << "include file";
    break;

  case CTRL_LABEL:
    os << token << ":";
    break;

  case TOK_COMMA:
    os << "','";
    break;
  case TOK_SEMICOLON:
    os << "';'";
    break;

  case CTRL_STATEMENTBEGIN:
    os << "[" << ( !token.empty() ? token : "--source not available--" ) << "]";
    break;
  case CTRL_PROGEND:
    os << "progend";
    break;
  case CTRL_MAKELOCAL:
    os << "makelocal";
    break;
  case CTRL_JSR_USERFUNC:
    os << "jmp userfunc @" << lval;
    break;
  case INS_CHECK_MRO:
    os << "check mro (this at offset " << lval << ")";
    break;
  case INS_POP_PARAM_BYREF:
    os << "pop param byref '" << token << "'";
    break;
  case INS_POP_PARAM:
    os << "pop param '" << token << "'";
    break;
  case INS_GET_ARG:
    os << "get arg '" << token << "'";
    break;
  case CTRL_LEAVE_BLOCK:
    os << "leave block(" << lval << ")";
    break;

  case INS_CALL_METHOD:
    os << "Call Method " << token << " (" << lval << " params)";
    break;
  case TOK_USERFUNC:
    os << "User Function " << ( !token.empty() ? token : "--function name not available--" );
    break;
  case TOK_FUNCREF:
    os << "Function Ref " << ( !token.empty() ? token : "--function name not available--" ) << "@"
       << lval;
    break;
  case TOK_UNPLUSPLUS:
    os << "unary ++";
    break;
  case TOK_UNMINUSMINUS:
    os << "unary --";
    break;
  case TOK_UNPLUSPLUS_POST:
    os << "unary ++ post";
    break;
  case TOK_UNMINUSMINUS_POST:
    os << "unary -- post";
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
    os << "set member id '" << getObjMember( lval )->code << "' unary ++";
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
    os << "set member id '" << getObjMember( lval )->code << "' unary --";
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
    os << "set member id '" << getObjMember( lval )->code << "' unary ++ post";
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    os << "set member id '" << getObjMember( lval )->code << "' unary -- post";
    break;
  case INS_SKIPIFTRUE_ELSE_CONSUME:
    os << "peek at top of stack; skip " << lval << " instructions if true, otherwise consume it";
    break;
  case RSV_COLON:
    os << "':'";
    break;
  case RSV_PROGRAM:
    os << "program";
    break;
  case RSV_ENDPROGRAM:
    os << "endprogram";
    break;
  case RSV_ENUM:
    os << "enum";
    break;
  case RSV_ENDENUM:
    os << "endenum";
    break;
  case RSV_ELVIS:
    os << "?: (elvis)";
    break;
  case TOK_INTERPOLATE_STRING:
    os << "interpolate string "
       << "(" << lval << "parts)";
    break;
  case TOK_FORMAT_EXPRESSION:
    os << "format expression";
    break;

  case INS_UNPACK_SEQUENCE:
  {
    int rest_index = lval > 0x7F ? ( ( lval >> 7 ) & 0x7F ) : -1;
    os << fmt::format( "unpack sequence ({} elements, rest index {})", lval & 0x7F, rest_index );
    break;
  }
  case INS_UNPACK_INDICES:
  {
    int rest_index = lval > 0x7F ? ( ( lval >> 7 ) & 0x7F ) : -1;
    os << fmt::format( "unpack indices ({} elements, rest index {})", lval & 0x7F, rest_index );
    break;
  }
  case INS_TAKE_GLOBAL:
    os << "take global #" << lval;
    break;
  case INS_TAKE_LOCAL:
    os << "take local #" << lval;
    break;

  case TOK_FUNC:
  {
    os << "Func(" << (int)module << "," << lval << "): ";
    if ( !token.empty() )
      os << token;
    else
      os << "<unknown>";
    return;
  }

  case TOK_SPREAD:
    os << ( lval ? "spread-into" : "create-spread" );
    break;

  case INS_LOGICAL_JUMP:
    os << fmt::format( "logical jump if {} to {}", type != TYP_LOGICAL_JUMP_FALSE, lval );
    break;
  case INS_LOGICAL_CONVERT:
    os << "logical convert";
    break;

  case TOK_REGEXP:
    os << "create-regular-expression";
    break;

  default:
    os << "Unknown Token: (" << int( id ) << "," << int( type );
    if ( !token.empty() )
      os << ",'" << token << "'";
    os << ")";
    break;
  }
}

std::ostream& operator<<( std::ostream& os, const Token& tok )
{
  tok.printOn( os );
  return os;
}
}  // namespace Bscript
}  // namespace Pol
