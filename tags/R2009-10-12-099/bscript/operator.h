/*
History
=======

Notes
=======

*/

#ifndef __OPERATOR_H
#define __OPERATOR_H

#include "tokens.h"

enum Precedence {
    PREC_PAREN = 14,
    PREC_UNARY_OPS = 12,
    PREC_MULT = 11,
    PREC_PLUS = 10,
    PREC_LESSTHAN = 9,
    PREC_BSLEFT = 8,
	PREC_BSRIGHT = 8,
	PREC_BITAND = 8,
    PREC_BITXOR = 7,
    PREC_BITOR = 6,
    PREC_EQUALTO = 5,
    PREC_LOGAND = 4,
    PREC_LOGOR = 3,
    PREC_ASSIGN = 1,
    PREC_COMMA = 0,
	PREC_DEPRECATED = 0,
    PREC_TERMINATOR=-1
};

typedef struct {
    char code[16];

    BTokenId id;
    Precedence precedence;
    BTokenType type;
    bool ambig; // if true, part of it matches part of another
	bool deprecated;
} Operator;

#endif
