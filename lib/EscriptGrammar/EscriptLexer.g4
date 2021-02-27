lexer grammar EscriptLexer;
channels { COMMENTS }

// Keywords

IF:                 'if';
THEN:               'then';
ELSEIF:             'elseif';
ENDIF:              'endif';
ELSE:               'else';
                    // '_OptionBracketed';
GOTO:               'goto';
// GOSUB:              'gosub'; // not used??
RETURN:             'return';
TOK_CONST:          'const';
VAR:                'var';
DO:                 'do';
DOWHILE:            'dowhile';
WHILE:              'while';
ENDWHILE:           'endwhile';
EXIT:               'exit';
FUNCTION:           'function';
ENDFUNCTION:        'endfunction';
EXPORTED:           'exported';
USE:                'use';
INCLUDE:            'include';
BREAK:              'break';
CONTINUE:           'continue';
FOR:                'for';
ENDFOR:             'endfor';
TO:                 'to';
// NEXT:               'next'; // not used?
FOREACH:            'foreach';
ENDFOREACH:         'endforeach';
REPEAT:             'repeat';
UNTIL:              'until';
PROGRAM:            'program';
ENDPROGRAM:         'endprogram';
CASE:               'case';
DEFAULT:            'default';
ENDCASE:            'endcase';
ENUM:               'enum';
ENDENUM:            'endenum';

// Reserved words (future)
DOWNTO:             'downto';
STEP:               'step';
REFERENCE:          'reference';
TOK_OUT:            'out';
INOUT:              'inout';
BYVAL:              'ByVal';
STRING:             'string';
TOK_LONG:           'long';
INTEGER:            'integer';
UNSIGNED:           'unsigned';
SIGNED:             'signed';
REAL:               'real';
FLOAT:              'float';
DOUBLE:             'double';
AS:                 'as';
IS:                 'is';

// Operators
AND_A:              '&&';
AND_B:              'and';
fragment AND:       '&&' | 'and';
OR_A:               '||';
OR_B:               'or';
fragment OR:        '||' | 'or';
BANG_A:              '!';
BANG_B:             'not';
fragment BANG:      '!' | 'not';
BYREF:              'byref';
UNUSED:             'unused';
TOK_ERROR:          'error';
HASH:               'hash';
DICTIONARY:         'dictionary';
STRUCT:             'struct';
ARRAY:              'array';
STACK:              'stack';
TOK_IN:             'in';

// Literals

DECIMAL_LITERAL:    ('0' | [1-9] Digits?);
HEX_LITERAL:        '0' [xX] HexDigits;
OCT_LITERAL:        '0' [0-7]+;
BINARY_LITERAL:     '0' [bB] [01]+;
                    
FLOAT_LITERAL:      (Digits '.' Digits? | '.' Digits) ExponentPart?
             |       Digits ExponentPart
             ;

HEX_FLOAT_LITERAL:  '0' [xX] (HexDigits '.'? | HexDigits? '.' HexDigits) [pP] [+-]? Digits;

STRING_LITERAL:     '"' (~[\\"] | EscapeSequence)* '"';

// Separators

LPAREN:             '(';
RPAREN:             ')';
LBRACK:             '[';
RBRACK:             ']';
LBRACE:             '{';
RBRACE:             '}';
DOT:                '.';
ARROW:              '->';
MUL:                '*';
DIV:                '/';
MOD:                '%';
ADD:                '+';
SUB:                '-';
ADD_ASSIGN:         '+=';
SUB_ASSIGN:         '-=';
MUL_ASSIGN:         '*=';
DIV_ASSIGN:         '/=';
MOD_ASSIGN:         '%=';
LE:                 '<=';
LT:                 '<';
GE:                 '>=';
GT:                 '>';
RSHIFT:             '>>';
LSHIFT:             '<<';
BITAND:             '&';
CARET:              '^';
BITOR:              '|';
NOTEQUAL_A:         '<>';
NOTEQUAL_B:         '!=';
fragment NOTEQUAL:  '<>' | '!=';
EQUAL_DEPRECATED:   '=';
EQUAL:              '==';
// && covered above
// || covered above
ASSIGN:            ':=';
ADDMEMBER:          '.+';
DELMEMBER:          '.-';
CHKMEMBER:          '.?';

SEMI:               ';';
COMMA:              ',';
TILDE:              '~';
AT:                 '@';

COLONCOLON:         '::';
COLON:              ':';
INC:                '++';
DEC:                '--';

ELVIS:              '?:';
// Whitespace and comments

WS:                 [ \t\r\n\u000C]+ -> channel(HIDDEN);
COMMENT:            '/*' .*? '*/'    -> channel(COMMENTS);
LINE_COMMENT:       '//' ~[\r\n]*    -> channel(COMMENTS);

// Identifiers

IDENTIFIER:         Letter LetterOrDigit*;

// Fragment rules

fragment ExponentPart
    : [eE] [+-]? Digits
    ;

// We currently allow all escapes, as they are checked during semantic analysis.
fragment EscapeSequence
    : '\\' [xX] HexDigit HexDigit
    | '\\' .
    ;

fragment HexDigits
    : HexDigit HexDigit*
    ;

fragment HexDigit
    : [0-9a-fA-F]
    ;

fragment Digits
    : [0-9] [0-9]*
    ;

fragment LetterOrDigit
    : Letter
    | [0-9]
    ;

fragment Letter
    : [a-zA-Z$_]
    | ~[\u0000-\u007F\uD800-\uDBFF] // covers all characters above 0x7F which are not a surrogate
    | [\uD800-\uDBFF] [\uDC00-\uDFFF] // covers UTF-16 surrogate pairs encodings for U+10000 to U+10FFFF
    ;

