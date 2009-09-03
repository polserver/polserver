/*
History
=======

Notes
=======

*/

#ifndef GP_GPHELP_H
#define GP_GPHELP_H

#include "tree.h"

class Token;
class GTokenFunctionDef;
class GTokenTerminalDef;

Token MakeFunctionToken( unsigned module, unsigned funcidx );
Token MakeDoubleToken( double value );

extern GTokenFunctionDef f_plus;
extern GTokenFunctionDef f_minus;
extern GTokenFunctionDef f_mult;
extern GTokenFunctionDef f_div;

extern GTokenFunctionDef f_EQ;
extern GTokenFunctionDef f_NE;
extern GTokenFunctionDef f_LT;
extern GTokenFunctionDef f_LE;
extern GTokenFunctionDef f_GT;
extern GTokenFunctionDef f_GE;

extern GTokenTerminalDef t_zero;
extern GTokenTerminalDef t_one;


extern GIfThenElseFunctionDef f_ifthenelse;
extern GWhileExprDoFunctionDef f_whileexprdo;
extern GMultiStatementFunctionDef f_prog2n;
extern GMultiStatementFunctionDef f_prog3n;

#endif
