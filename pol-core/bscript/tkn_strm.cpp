/** @file
 *
 * @par History
 */

#include "objmembers.h"
#include "objmethods.h"
#include "token.h"
#include "tokens.h"

namespace Pol::Bscript
{
std::string Token::tostring() const
{
  switch ( id )
  {
  case TOK_LONG:
    return fmt::format( "{}L", lval );
  case TOK_DOUBLE:
    return fmt::format( "{}LF", dval );
  case TOK_BOOL:
    return fmt::format( "{} (boolean)", static_cast<bool>( lval ) );
  case TOK_IDENT:
    return token;
  case INS_ADDMEMBER2:
    return fmt::format( "addmember({})", token );
  case INS_ADDMEMBER_ASSIGN:
    return fmt::format( "addmember-assign({})", token );
  case TOK_STRING:
    return fmt::format( "\"{}\"", token );
  case TOK_LOCALVAR:
    return fmt::format( "local #{} {}", lval,
                        token.empty() ? std::string() : fmt::format( " ({})", token ) );
  case TOK_GLOBALVAR:
    return fmt::format( "global #{} {}", lval,
                        token.empty() ? std::string() : fmt::format( " ({})", token ) );
  case TOK_ERROR:
    return "error";
  case TOK_DICTIONARY:
    return "dictionary";
  case TOK_MULT:
    return "*";
  case TOK_DIV:
    return "/";
  case TOK_ADD:
    return "+";

  case TOK_INSERTINTO:
    return "init{}";
  case TOK_SUBTRACT:
    return "-";
  case TOK_PLUSEQUAL:
    return "+=";
  case TOK_MINUSEQUAL:
    return "-=";
  case TOK_TIMESEQUAL:
    return "*=";
  case TOK_DIVIDEEQUAL:
    return "/=";
  case TOK_MODULUSEQUAL:
    return "%=";

  case TOK_ASSIGN:
    return ":=";
  case INS_ASSIGN_LOCALVAR:
    return fmt::format( "local{} {} :=", lval,
                        token.empty() ? std::string() : fmt::format( " ({})", token ) );
  case INS_ASSIGN_GLOBALVAR:
    return fmt::format( "global{} {} :=", lval,
                        token.empty() ? std::string() : fmt::format( " ({})", token ) );
  case INS_ASSIGN_CONSUME:
    return ":= #";
  case INS_SUBSCRIPT_ASSIGN_CONSUME:
    return fmt::format( "[] := ({}) #", lval );
  case INS_SUBSCRIPT_ASSIGN:
    return fmt::format( "[] := ({})", lval );
  case TOK_LESSTHAN:
    return "<";
  case TOK_LESSEQ:
    return "<=";
  case TOK_GRTHAN:
    return ">";
  case TOK_GREQ:
    return ">=";
  case TOK_EQUAL1:
    return "=";
  case TOK_EQUAL:
    return "==";
  case TOK_NEQ:
    return "<>";
  case TOK_AND:
    return "&&";
  case TOK_OR:
    return "||";
  case TOK_ARRAY_SUBSCRIPT:
    return fmt::format( "[] {}", lval );
  case INS_MULTISUBSCRIPT:
    return fmt::format( "[{:,<{}}]", "", lval - 1 );
  case INS_MULTISUBSCRIPT_ASSIGN:
    return fmt::format( "[{:,<{}}] :=", "", lval - 1 );
  case TOK_ADDMEMBER:
    return ".+";
  case TOK_DELMEMBER:
    return ".-";
  case TOK_CHKMEMBER:
    return ".?";
  case TOK_MEMBER:
    return ".";
  case INS_GET_MEMBER:
    return fmt::format( "get member '{}'", token );
  case INS_SET_MEMBER:
    return fmt::format( "set member '{}'", token );
  case INS_SET_MEMBER_CONSUME:
    return fmt::format( "set member '{}' #", token );
  case INS_GET_MEMBER_ID:
    return fmt::format( "get member id '{}' ({})", getObjMember( lval )->code, lval );
    break;
  case INS_SET_MEMBER_ID:
    return fmt::format( "set member id '{}' ({})", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME:
    return fmt::format( "set member id '{}' ({}) #", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
    return fmt::format( "set member id '{}' ({})  += #", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
    return fmt::format( "set member id '{}' ({})  -= #", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
    return fmt::format( "set member id '{}' ({})  *= #", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
    return fmt::format( "set member id '{}' ({})  /= #", getObjMember( lval )->code, lval );
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
    return fmt::format( "set member id '{}' ({})  %= #", getObjMember( lval )->code, lval );

  case INS_CALL_METHOD_ID:
    return fmt::format( "Call Method id {} (#{}, {} params)", getObjMethod( lval )->code, lval,
                        type );
  case TOK_IN:
    return "in";
  case INS_DICTIONARY_ADDMEMBER:
    return "add dictionary member";

  case TOK_UNPLUS:
    return "unary +";
  case TOK_UNMINUS:
    return "unary -";
  case TOK_LOG_NOT:
    return "!";
  case TOK_CONSUMER:
    return "#";
  case TOK_REFTO:
    return "refto";
  case TOK_UNUSED:
    return "unused";
  case TOK_BITAND:
    return "&";
  case TOK_BITOR:
    return "|";
  case TOK_BSRIGHT:
    return ">>";
  case TOK_BSLEFT:
    return "<<";
  case TOK_BITXOR:
    return "^";
  case TOK_BITWISE_NOT:
    return "~";
  case TOK_MODULUS:
    return "%";

  case INS_INITFOREACH:
    return fmt::format( "initforeach @{}", lval );
  case INS_STEPFOREACH:
    return fmt::format( "stepforeach @{}", lval );
  case INS_CASEJMP:
    return "casejmp";
  case INS_INITFOR:
    return fmt::format( "initfor @{}", lval );
  case INS_NEXTFOR:
    return fmt::format( "nextfor @{}", lval );

  case TOK_TERM:
    return "Terminator";

  case TOK_LPAREN:
    return "(";
  case TOK_RPAREN:
    return ")";
  case TOK_LBRACKET:
    return "[";
  case TOK_RBRACKET:
    return "]";
  case TOK_LBRACE:
    return "{";
  case TOK_RBRACE:
    return "}";

  case RSV_JMPIFTRUE:
    return fmt::format( "if true goto {}", lval );
  case RSV_JMPIFFALSE:
    return fmt::format( "if false goto {}", lval );
  case RSV_ST_IF:
    return "if";
  case RSV_GOTO:
    return fmt::format( "goto {}", lval );
  case RSV_GOSUB:
    return fmt::format( "gosub{}", lval );
  case RSV_EXIT:
    return "exit";
  case RSV_RETURN:
    return "return";
  case RSV_LOCAL:
    return fmt::format( "decl local #{}", lval );
  case RSV_GLOBAL:
    return fmt::format( "decl global #{}", lval );
  case RSV_VAR:
    return "var";
  case RSV_CONST:
    return "const";
  case RSV_FUNCTION:
    return "function";
  case RSV_ENDFUNCTION:
    return "endfunction";
  case RSV_DO:
    return "do";
  case RSV_DOWHILE:
    return "dowhile";
  case RSV_WHILE:
    return "while";
  case RSV_ENDWHILE:
    return "endwhile";
  case RSV_REPEAT:
    return "repeat";
  case RSV_UNTIL:
    return "until";
  case RSV_FOR:
    return "for";
  case RSV_ENDFOR:
    return "endfor";
  case RSV_FOREACH:
    return "foreach";
  case RSV_ENDFOREACH:
    return "endforeach";
  case INS_DECLARE_ARRAY:
    return "declare array";
  case TOK_ARRAY:
    return "array";
  case TOK_STRUCT:
    return "struct";
  case TOK_CLASSINST:
    return fmt::format( "class instance #{}", lval );
  case INS_UNINIT:
    return "uninit";
  case RSV_USE_MODULE:
    return "use module";
  case RSV_INCLUDE_FILE:
    return "include file";

  case CTRL_LABEL:
    return fmt::format( "{}:", token );

  case TOK_COMMA:
    return "','";
  case TOK_SEMICOLON:
    return "';'";

  case CTRL_STATEMENTBEGIN:
    return fmt::format( "[{}]", !token.empty() ? token : "--source not available--" );
  case CTRL_PROGEND:
    return "progend";
  case CTRL_MAKELOCAL:
    return "makelocal";
  case CTRL_JSR_USERFUNC:
    return fmt::format( "jmp userfunc @{}", lval );
  case INS_CHECK_MRO:
    return fmt::format( "check mro (this at offset {})", lval );
  case INS_POP_PARAM_BYREF:
    return fmt::format( "pop param byref '{}'", token );
  case INS_POP_PARAM:
    return fmt::format( "pop param '{}'", token );
  case INS_GET_ARG:
    return fmt::format( "get arg '{}'", token );
  case CTRL_LEAVE_BLOCK:
    return fmt::format( "leave block({})", lval );

  case INS_CALL_METHOD:
    return fmt::format( "Call Method {} ({} params) ", token, lval );
    break;
  case TOK_USERFUNC:
    return fmt::format( "User Function {}",
                        !token.empty() ? token : "--function name not available--" );
    break;
  case TOK_FUNCREF:
    return fmt::format( "Function Ref {}@{}",
                        !token.empty() ? token : "--function name not available--", lval );
  case TOK_UNPLUSPLUS:
    return "unary ++";
  case TOK_UNMINUSMINUS:
    return "unary --";
  case TOK_UNPLUSPLUS_POST:
    return "unary ++ post";
  case TOK_UNMINUSMINUS_POST:
    return "unary -- post";
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
    return fmt::format( "set member id '{}' unary ++", getObjMember( lval )->code );
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
    return fmt::format( "set member id '{}' unary --", getObjMember( lval )->code );
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
    return fmt::format( "set member id '{}' unary ++ post", getObjMember( lval )->code );
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    return fmt::format( "set member id '{}' unary -- post", getObjMember( lval )->code );
  case INS_SKIPIFTRUE_ELSE_CONSUME:
    return fmt::format( "peek at top of stack; skip {} instructions if true, otherwise consume it",
                        lval );
  case RSV_COLON:
    return "':'";
  case RSV_PROGRAM:
    return "program";
  case RSV_ENDPROGRAM:
    return "endprogram";
  case RSV_ENUM:
    return "enum";
  case RSV_ENDENUM:
    return "endenum";
  case RSV_ELVIS:
    return "?: (elvis)";
  case TOK_INTERPOLATE_STRING:
    return fmt::format( "interpolate string ({} parts)", lval );
  case TOK_FORMAT_EXPRESSION:
    return "format expression";

  case INS_UNPACK_SEQUENCE:
  {
    int rest_index = lval > 0x7F ? ( ( lval >> 7 ) & 0x7F ) : -1;
    return fmt::format( "unpack sequence ({} elements, rest index {})", lval & 0x7F, rest_index );
  }
  case INS_UNPACK_INDICES:
  {
    int rest_index = lval > 0x7F ? ( ( lval >> 7 ) & 0x7F ) : -1;
    return fmt::format( "unpack indices ({} elements, rest index {})", lval & 0x7F, rest_index );
  }
  case INS_TAKE_GLOBAL:
    return fmt::format( "take global #{}", lval );
  case INS_TAKE_LOCAL:
    return fmt::format( "take local #{}", lval );

  case TOK_FUNC:
    return fmt::format( "Func({},{}): {}", (int)module, lval,
                        !token.empty() ? token : "<unknown>" );

  case TOK_SPREAD:
    return ( lval ? "spread-into" : "create-spread" );

  case INS_LOGICAL_JUMP:
    return fmt::format( "logical jump if {} to {}", type != TYP_LOGICAL_JUMP_FALSE, lval );
  case INS_LOGICAL_CONVERT:
    return "logical convert";

  case TOK_REGEXP:
    return "create-regular-expression";

  default:
    return fmt::format( "Unknown Token: ({},{}{})", id, type,
                        token.empty() ? std::string{} : fmt::format( ",'{}'", token ) );
  }
}
}  // namespace Pol::Bscript

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Token>::format(
    const Pol::Bscript::Token& t, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format( t.tostring(), ctx );
}
