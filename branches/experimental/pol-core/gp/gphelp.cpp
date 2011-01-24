/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../bscript/token.h"

#include "gphelp.h"
#include "tree.h"

Token MakeFunctionToken( unsigned module, unsigned funcidx )
{
    Token tkn;

    tkn.module = static_cast<unsigned char>(module);
    tkn.type = TYP_FUNC;
    tkn.id = TOK_FUNC;
    tkn.lval = funcidx;

    return tkn;
}

Token MakeLongToken( int value )
{
    Token tkn;

    tkn.module = Mod_Basic;
    tkn.type = TYP_OPERAND;
    tkn.id = TOK_LONG;
    tkn.lval = value;

    return tkn;
}

Token MakeDoubleToken( double value )
{
    Token tkn;

    tkn.module = Mod_Basic;
    tkn.type = TYP_OPERAND;
    tkn.id = TOK_DOUBLE;
    tkn.dval = value;

    return tkn;
}

Token MakeBinaryOpToken( BTokenId id )
{
    Token tkn;
    
    tkn.module = Mod_Basic;
    tkn.type = TYP_OPERATOR;
    tkn.id = id;
    tkn.lval = 0;
    
    return tkn;
}

GTokenFunctionDef f_plus( "+", 2, MakeBinaryOpToken( TOK_ADD ) );
GTokenFunctionDef f_minus( "-", 2, MakeBinaryOpToken( TOK_SUBTRACT ) );
GTokenFunctionDef f_mult( "*", 2, MakeBinaryOpToken( TOK_MULT ) );
GTokenFunctionDef f_div( "/", 2, MakeBinaryOpToken( TOK_DIV ) );

GTokenFunctionDef f_EQ( "EQ", 2, MakeBinaryOpToken( TOK_EQUAL ) );
GTokenFunctionDef f_NE( "NE", 2, MakeBinaryOpToken( TOK_NEQ ) );
GTokenFunctionDef f_LT( "LT", 2, MakeBinaryOpToken( TOK_LESSTHAN ) );
GTokenFunctionDef f_LE( "LE", 2, MakeBinaryOpToken( TOK_LESSEQ ) );
GTokenFunctionDef f_GT( "GT", 2, MakeBinaryOpToken( TOK_GRTHAN ) );
GTokenFunctionDef f_GE( "GE", 2, MakeBinaryOpToken( TOK_GREQ ) );

GTokenTerminalDef t_zero( "0", MakeLongToken( 0 ) );
GTokenTerminalDef t_one( "1", MakeLongToken( 1 ) );
