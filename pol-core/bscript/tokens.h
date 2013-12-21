/*
History
=======

Notes
=======

*/

#ifndef __TOKENS_H
#define __TOKENS_H
namespace Pol {
  namespace Bscript {
	enum BTokenType
	{
	  TYP_TERMINATOR,
	  TYP_OPERAND,
	  TYP_OPERATOR,			 // BINARY implied
	  TYP_UNARY_OPERATOR,
	  TYP_LEFTPAREN,
	  TYP_RIGHTPAREN,
	  TYP_LEFTBRACKET,
	  TYP_RIGHTBRACKET,

	  TYP_TESTMAX = TYP_RIGHTBRACKET,

	  TYP_RESERVED,

	  TYP_LABEL,   // a GOTO/GOSUB label

	  TYP_FUNC,	// func returning something
	  TYP_METHOD,  // object method

	  TYP_USERFUNC,

	  TYP_SEPARATOR,
	  TYP_DELIMITER,

	  TYP_CONTROL,

	  TYP_LEFTBRACE,
	  TYP_RIGHTBRACE,

	  TYP_NUMTYPES
	};

	enum BTokenId
	{
	  TOK_LONG,											   // 0
	  TOK_DOUBLE,
	  TOK_STRING,   //  "string literal"

	  TOK_IDENT, // variable identifier, i.e. A, AB, A$ */

	  TOK_ADD,
	  TOK_SUBTRACT,										   // 5
	  TOK_MULT,
	  TOK_DIV,

	  TOK_ASSIGN,
	  INS_ASSIGN_CONSUME,

	  TOK_PLUSEQUAL,
	  TOK_MINUSEQUAL,
	  TOK_TIMESEQUAL,
	  TOK_DIVIDEEQUAL,
	  TOK_MODULUSEQUAL,
	  TOK_INSERTINTO,

	  /* comparison operators */
	  TOK_LESSTHAN,
	  TOK_LESSEQ,											 // 15
	  TOK_GRTHAN,
	  TOK_GREQ,

	  TOK_AND,
	  TOK_OR,

	  /* equalite/inequality operators */
	  TOK_EQUAL,											  // 20
	  TOK_NEQ,

	  /* unary operators */
	  TOK_UNPLUS,
	  TOK_UNMINUS,
	  TOK_LOG_NOT,
	  TOK_BITWISE_NOT,										// 25

	  TOK_CONSUMER,

	  TOK_ARRAY_SUBSCRIPT,

	  TOK_ADDMEMBER,
	  TOK_DELMEMBER,
	  TOK_CHKMEMBER,										  // 30

	  CTRL_STATEMENTBEGIN,
	  CTRL_PROGEND,
	  CTRL_MAKELOCAL,
	  CTRL_JSR_USERFUNC,
	  INS_POP_PARAM,										  // 35
	  CTRL_LEAVE_BLOCK,		// offset is number of variables to remove

	  RSV_JMPIFFALSE,
	  RSV_JMPIFTRUE,

	  RSV_GOTO,
	  RSV_RETURN,											 // 40
	  RSV_EXIT,

	  RSV_LOCAL,
	  RSV_GLOBAL,
	  RSV_VAR,

	  RSV_FUNCTION,										   // 45

	  INS_DECLARE_ARRAY,

	  TOK_FUNC,
	  TOK_USERFUNC,
	  TOK_ERROR,
	  TOK_IN,												 // 50
	  TOK_LOCALVAR,
	  TOK_GLOBALVAR,
	  INS_INITFOREACH,
	  INS_STEPFOREACH,
	  INS_CASEJMP,											// 55
	  INS_GET_ARG,
	  TOK_ARRAY,

	  INS_CALL_METHOD,

	  TOK_DICTIONARY,										 // 60
	  TOK_STACK,
	  INS_INITFOR,
	  INS_NEXTFOR,
	  TOK_REFTO,
	  INS_POP_PARAM_BYREF,									// 65
	  TOK_MODULUS,

	  TOK_BSLEFT,
	  TOK_BSRIGHT,
	  TOK_BITAND,
	  TOK_BITOR,
	  TOK_BITXOR,

	  TOK_STRUCT,											 // 70
	  INS_SUBSCRIPT_ASSIGN,
	  INS_SUBSCRIPT_ASSIGN_CONSUME,
	  INS_MULTISUBSCRIPT,
	  INS_MULTISUBSCRIPT_ASSIGN,
	  INS_ASSIGN_LOCALVAR,									// 75
	  INS_ASSIGN_GLOBALVAR,

	  INS_GET_MEMBER,
	  INS_SET_MEMBER,
	  INS_SET_MEMBER_CONSUME,

	  INS_ADDMEMBER2,										 // 80
	  INS_ADDMEMBER_ASSIGN,
	  INS_UNINIT,
	  INS_DICTIONARY_ADDMEMBER,

	  INS_GET_MEMBER_ID,
	  INS_SET_MEMBER_ID,									  // 85
	  INS_SET_MEMBER_ID_CONSUME,
	  INS_CALL_METHOD_ID,

	  TOK_EQUAL1,

	  INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL,
	  INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL,
	  INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL,
	  INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL,
	  INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL,

	  // TokenIds that aren't part of emitted code:

	  TOK_SEMICOLON = 256,
	  TOK_COMMA,
	  TOK_LPAREN,							 // used only by parser
	  TOK_RPAREN,
	  TOK_TERM,						// ID 24  // terminator, ';', etc.
	  TOK_LBRACKET,
	  TOK_RBRACKET,
	  TOK_LBRACE,
	  TOK_RBRACE,

	  RSV_FOREACH,
	  RSV_ENDFOREACH,		 // RSV_IN: use TOK_IN

	  RSV_DECLARE,
	  RSV_FUTURE,
	  RSV_BREAK,
	  RSV_CONTINUE,		// 270
	  RSV_USE_MODULE,
	  RSV_INCLUDE_FILE,

	  RSV_FOR,
	  RSV_NEXT,
	  RSV_TO,
	  RSV_STEP,
	  RSV_THEN,
	  RSV_ST_IF,
	  RSV_ELSE,
	  RSV_ELSEIF,		   // 280
	  RSV_ENDIF,
	  RSV_GOSUB,
	  RSV_BEGIN,
	  RSV_ENDB,
	  RSV_DO,
	  RSV_WHILE,
	  RSV_OPTION_BRACKETED,

	  CTRL_LABEL,	// LABEL:
	  CTRL_NOTHING,
	  RSV_CONST,			// 290
	  RSV_ENDWHILE,
	  RSV_REPEAT,
	  RSV_UNTIL,
	  RSV_ENDFOR,
	  RSV_ENDFUNCTION,

	  RSV_SWITCH,
	  RSV_CASE,
	  RSV_DEFAULT,
	  RSV_ENDSWITCH,

	  RSV_COLON,			 // 300
	  RSV_PROGRAM,
	  RSV_ENDPROGRAM,
	  RSV_ENUM,
	  RSV_ENDENUM,
	  RSV_EXPORTED,
	  TOK_MEMBER,
	  TOK_DICTKEY,
	  RSV_DOWHILE
	};

	enum ESCRIPT_CASE_TYPES
	{
	  CASE_TYPE_LONG = 255,
	  CASE_TYPE_DEFAULT = 254,
	  CASE_STRING_MAXLEN = 253
	};
  }
}
#endif
