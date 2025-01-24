/** @file
 *
 * @par History
 */


#ifndef __TOKENS_H
#define __TOKENS_H

#include <fmt/format.h>
#include <iosfwd>

#include "../clib/rawtypes.h"
#include "../clib/strutil.h"

namespace Pol
{
namespace Bscript
{
/**
 * The token type
 *
 * This also gets outputted to the compiled bytecode.
 * Using hardcoded int conversion for better backward compatibility: every
 * insertion in the middle of it done in the past shifted forward all the bytes
 *
 * @note 20151229 Bodom
 *       This seems to be the first byte on the outputted bytecode, but not always.
 *       It looks like over time it also assumed different meaning, like saying the
 *       number of parameters in a method call. Type names may be out to date.
 */
enum BTokenType : u8
{
  TYP_TERMINATOR = 0x00,
  TYP_OPERAND = 0x01,
  TYP_OPERATOR = 0x02,  // BINARY implied
  TYP_UNARY_OPERATOR = 0x03,
  TYP_LEFTPAREN = 0x04,
  TYP_RIGHTPAREN = 0x05,
  TYP_LEFTBRACKET = 0x06,
  TYP_RIGHTBRACKET = 0x07,

  TYP_TESTMAX = TYP_RIGHTBRACKET,

  TYP_RESERVED = 0x08,

  TYP_LABEL = 0x09,  // a GOTO/GOSUB label

  TYP_FUNC = 0x10,    // func returning something
  TYP_METHOD = 0x1a,  // object method

  TYP_USERFUNC = 0x1b,

  TYP_SEPARATOR = 0x1c,
  TYP_DELIMITER = 0x1d,

  TYP_CONTROL = 0x1e,

  TYP_LEFTBRACE = 0x1f,
  TYP_RIGHTBRACE = 0x20,

  TYP_NUMTYPES = 0x21,
  TYP_FUNCREF = 0x22,
  TYP_UNARY_PLACEHOLDER = 0x23,
};


/**
 * The token ID: what this token is
 *
 * This will be outputted on the compiled bytecode as 2nd byte.
 * Tokens above 0xFF cannot be outputted and are used only internally
 * by the compiler: please remember to allocate your new token in the
 * upper space if it is not meant to be outputted.
 *
 * Using hardcoded int conversion for better backward compatibility: every
 * insertion in the middle of it done in the past shifted forward all the
 * tokens, invalidating bytecode documentation.
 */
enum BTokenId : u16
{
  // --- LOWER SPACE 0x00-0xFF: TOKENS OUTPUTTED TO THE BYTECODE ---
  TOK_LONG = 0x00,
  TOK_DOUBLE = 0x01,
  TOK_STRING = 0x02,  //  "string literal"

  TOK_IDENT = 0x03,  // variable identifier, i.e. A, AB, A$ */

  TOK_ADD = 0x04,
  TOK_SUBTRACT = 0x05,
  TOK_MULT = 0x06,
  TOK_DIV = 0x07,

  TOK_ASSIGN = 0x08,
  INS_ASSIGN_CONSUME = 0x09,

  TOK_PLUSEQUAL = 0x0a,
  TOK_MINUSEQUAL = 0x0b,
  TOK_TIMESEQUAL = 0x0c,
  TOK_DIVIDEEQUAL = 0x0d,
  TOK_MODULUSEQUAL = 0x0e,
  TOK_INSERTINTO = 0x0f,

  // comparison operators
  TOK_LESSTHAN = 0x10,
  TOK_LESSEQ = 0x11,
  TOK_GRTHAN = 0x12,
  TOK_GREQ = 0x13,

  TOK_AND = 0x14,
  TOK_OR = 0x15,

  // equalite/inequality operators */
  TOK_EQUAL = 0x16,
  TOK_NEQ = 0x17,

  // unary operators
  TOK_UNPLUS = 0x18,
  TOK_UNMINUS = 0x19,
  TOK_LOG_NOT = 0x1a,
  TOK_BITWISE_NOT = 0x1b,

  TOK_CONSUMER = 0x1c,

  TOK_ARRAY_SUBSCRIPT = 0x1d,

  TOK_ADDMEMBER = 0x1e,
  TOK_DELMEMBER = 0x1f,
  TOK_CHKMEMBER = 0x20,

  CTRL_STATEMENTBEGIN = 0x21,
  CTRL_PROGEND = 0x22,
  CTRL_MAKELOCAL = 0x23,
  CTRL_JSR_USERFUNC = 0x24,
  INS_POP_PARAM = 0x25,
  CTRL_LEAVE_BLOCK = 0x26,  // offset is number of variables to remove

  RSV_JMPIFFALSE = 0x27,
  RSV_JMPIFTRUE = 0x28,

  RSV_GOTO = 0x29,
  RSV_RETURN = 0x2a,
  RSV_EXIT = 0x2b,

  RSV_LOCAL = 0x2c,
  RSV_GLOBAL = 0x2d,
  RSV_VAR = 0x2e,

  RSV_FUNCTION = 0x2f,

  INS_DECLARE_ARRAY = 0x30,

  TOK_FUNC = 0x31,
  TOK_USERFUNC = 0x32,
  TOK_ERROR = 0x33,
  TOK_IN = 0x34,
  TOK_LOCALVAR = 0x35,
  TOK_GLOBALVAR = 0x36,
  INS_INITFOREACH = 0x37,
  INS_STEPFOREACH = 0x38,
  INS_CASEJMP = 0x39,
  INS_GET_ARG = 0x3a,
  TOK_ARRAY = 0x3b,

  INS_CALL_METHOD = 0x3c,

  TOK_DICTIONARY = 0x3d,
  TOK_STACK = 0x3e,
  INS_INITFOR = 0x3f,
  INS_NEXTFOR = 0x40,
  TOK_REFTO = 0x41,
  INS_POP_PARAM_BYREF = 0x42,
  TOK_MODULUS = 0x43,

  TOK_BSLEFT = 0x44,
  TOK_BSRIGHT = 0x45,
  TOK_BITAND = 0x46,
  TOK_BITOR = 0x47,
  TOK_BITXOR = 0x48,

  TOK_STRUCT = 0x49,
  INS_SUBSCRIPT_ASSIGN = 0x4a,
  INS_SUBSCRIPT_ASSIGN_CONSUME = 0x4b,
  INS_MULTISUBSCRIPT = 0x4c,
  INS_MULTISUBSCRIPT_ASSIGN = 0x4d,
  INS_ASSIGN_LOCALVAR = 0x4e,
  INS_ASSIGN_GLOBALVAR = 0x4f,

  INS_GET_MEMBER = 0x50,
  INS_SET_MEMBER = 0x51,
  INS_SET_MEMBER_CONSUME = 0x52,

  INS_ADDMEMBER2 = 0x53,
  INS_ADDMEMBER_ASSIGN = 0x54,
  INS_UNINIT = 0x55,
  INS_DICTIONARY_ADDMEMBER = 0x56,

  INS_GET_MEMBER_ID = 0x57,
  INS_SET_MEMBER_ID = 0x58,
  INS_SET_MEMBER_ID_CONSUME = 0x59,
  INS_CALL_METHOD_ID = 0x5a,

  TOK_EQUAL1 = 0x5b,

  INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL = 0x5c,
  INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL = 0x5d,
  INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL = 0x5e,
  INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL = 0x5f,
  INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL = 0x60,

  TOK_FUNCREF = 0x61,

  TOK_UNPLUSPLUS = 0x62,
  TOK_UNMINUSMINUS = 0x63,
  TOK_UNPLUSPLUS_POST = 0x64,
  TOK_UNMINUSMINUS_POST = 0x65,
  INS_SET_MEMBER_ID_UNPLUSPLUS = 0x66,
  INS_SET_MEMBER_ID_UNMINUSMINUS = 0x67,
  INS_SET_MEMBER_ID_UNPLUSPLUS_POST = 0x68,
  INS_SET_MEMBER_ID_UNMINUSMINUS_POST = 0x69,
  INS_SKIPIFTRUE_ELSE_CONSUME = 0x6a,
  TOK_INTERPOLATE_STRING = 0x6b,
  TOK_FORMAT_EXPRESSION = 0x6c,
  TOK_BOOL = 0x6d,
  TOK_FUNCTOR = 0x6e,
  TOK_SPREAD = 0x70,
  TOK_CLASSINST = 0x71,
  TOK_CONSTRUCTINST = 0x72,
  INS_CHECK_MRO = 0x73,
  TOK_IS = 0x74,

  // --- UPPER SPACE 0x0100-0xFFFF: TOKENS THAT AREN'T PART OF EMITTED CODE ---
  //
  // these can be safely renumbered any time, but must start from 0x100

  TOK_SEMICOLON = 0x100,
  TOK_COMMA,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_TERM,  // terminator, ';', etc.
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_LBRACE,
  TOK_RBRACE,

  RSV_FOREACH,
  RSV_ENDFOREACH,  // RSV_IN: use TOK_IN

  RSV_FUTURE,
  RSV_BREAK,
  RSV_CONTINUE,
  RSV_USE_MODULE,
  RSV_INCLUDE_FILE,

  RSV_FOR,
  RSV_NEXT,
  RSV_TO,
  RSV_STEP,
  RSV_THEN,
  RSV_ST_IF,
  RSV_ELSE,
  RSV_ELSEIF,
  RSV_ENDIF,
  RSV_GOSUB,
  RSV_BEGIN,
  RSV_ENDB,
  RSV_DO,
  RSV_WHILE,
  RSV_OPTION_BRACKETED,

  CTRL_LABEL,  // LABEL:
  CTRL_NOTHING,
  RSV_CONST,
  RSV_ENDWHILE,
  RSV_REPEAT,
  RSV_UNTIL,
  RSV_ENDFOR,
  RSV_ENDFUNCTION,

  RSV_SWITCH,
  RSV_CASE,
  RSV_DEFAULT,
  RSV_ENDSWITCH,

  RSV_COLON,
  RSV_PROGRAM,
  RSV_ENDPROGRAM,
  RSV_ENUM,
  RSV_ENDENUM,
  RSV_EXPORTED,
  TOK_MEMBER,
  TOK_DICTKEY,
  RSV_DOWHILE,
  RSV_ELVIS,

  TOK_UNUSED,
};

enum ESCRIPT_CASE_TYPES : u8
{
  CASE_TYPE_LONG = 255,
  CASE_TYPE_DEFAULT = 254,
  CASE_TYPE_STRING = 253,
  CASE_TYPE_BOOL = 252,
  CASE_TYPE_UNINIT = 251
};
inline auto format_as( BTokenType t )
{
  return fmt::underlying( t );
}
inline auto format_as( BTokenId t )
{
  return fmt::underlying( t );
}
}  // namespace Bscript
}  // namespace Pol
#endif
