parser grammar EscriptParser;

options { tokenVocab=EscriptLexer; }

@header
{
}

@parser::members
{
}

compilationUnit
    : topLevelDeclaration* EOF
    ;

moduleUnit
    : moduleDeclarationStatement* EOF
    ;

moduleDeclarationStatement
    : moduleFunctionDeclaration
    | constStatement
    ;

moduleFunctionDeclaration
    : IDENTIFIER '(' moduleFunctionParameterList? ')' ';'
    ;

moduleFunctionParameterList
    : moduleFunctionParameter (',' moduleFunctionParameter)*
    ;

moduleFunctionParameter
    : IDENTIFIER (':=' expression)?
    ;

topLevelDeclaration
    : useDeclaration
    | includeDeclaration
    | programDeclaration
    | functionDeclaration
    | statement
    ;


functionDeclaration
    : EXPORTED? FUNCTION IDENTIFIER functionParameters block ENDFUNCTION
    ;

stringIdentifier
    : STRING_LITERAL
    | IDENTIFIER
    ;

useDeclaration
    : USE stringIdentifier ';'
    ;

includeDeclaration
    : INCLUDE stringIdentifier ';'
    ;

programDeclaration
    : PROGRAM IDENTIFIER programParameters block ENDPROGRAM
    ;

// Some ignored / to-be-handled things:
//  - Labels can only come before DO, WHILE, FOR, FOREACH, REPEAT, and CASE statements.
//  - Const expression must be optimizable

// TODO maybe split these all into individual statements?
statement
    : ifStatement
    | gotoStatement
    | returnStatement
    | constStatement
    | varStatement
    | doStatement
    | whileStatement
    | exitStatement
    | breakStatement
    | continueStatement
    | forStatement
    | foreachStatement
    | repeatStatement
    | caseStatement
    | enumStatement
    | SEMI
    | statementExpression=expression ';'
    ;

statementLabel
    : IDENTIFIER ':'
    ;

ifStatement
    : IF parExpression THEN? block (ELSEIF parExpression block)* (ELSE block)? ENDIF
    ;

gotoStatement
    : GOTO IDENTIFIER ';'
    ;

returnStatement
    : RETURN expression? ';'
    ;

constStatement
    : TOK_CONST variableDeclaration ';'
    ;

varStatement
    : VAR variableDeclarationList ';'
    ;

doStatement
    : statementLabel? DO block DOWHILE parExpression ';'
    ;

whileStatement
    : statementLabel? WHILE parExpression block ENDWHILE
    ;

exitStatement
    : EXIT ';'
    ;

breakStatement
    : BREAK IDENTIFIER? ';'
    ;

continueStatement
    : CONTINUE IDENTIFIER? ';'
    ;

forStatement
    : statementLabel? FOR forGroup ENDFOR
    ;

foreachIterableExpression
    : functionCall
    | scopedFunctionCall
    | IDENTIFIER
    | parExpression
    | bareArrayInitializer
    | explicitArrayInitializer
    ;

foreachStatement
    : statementLabel? FOREACH IDENTIFIER TOK_IN foreachIterableExpression block ENDFOREACH
    ;

repeatStatement
    : statementLabel? REPEAT block UNTIL expression ';'
    ;

caseStatement
    : statementLabel? CASE '(' expression ')' switchBlockStatementGroup+ ENDCASE
    ;

enumStatement
    : ENUM IDENTIFIER enumList ENDENUM
    ;

block
    : statement*
    ;

variableDeclarationInitializer
    : ':=' expression
    | '=' expression { notifyErrorListeners("Unexpected token: '='. Did you mean := for assign?\n"); }
    | ARRAY
    ;

enumList
    : enumListEntry (',' enumListEntry)* ','?
    ;

enumListEntry
    : IDENTIFIER (':=' expression)?
    ;

switchBlockStatementGroup
    : switchLabel+ block
    ;

switchLabel
    : (integerLiteral | IDENTIFIER | STRING_LITERAL) ':'
    | DEFAULT ':'
    ;

forGroup
    : cstyleForStatement 
    | basicForStatement
    ;

basicForStatement
    : IDENTIFIER ':=' expression TO expression block 
    ;

cstyleForStatement
    : '(' expression ';' expression ';' expression ')' block
    ;
identifierList
    : IDENTIFIER (',' identifierList)?
    ;
variableDeclarationList
    : variableDeclaration (',' variableDeclaration)*
    ;

variableDeclaration
    : IDENTIFIER variableDeclarationInitializer?
    ;

// PARAMETERS
programParameters
    : '(' programParameterList? ')'
    ;

programParameterList
    : programParameter (','? programParameter)*
    ;

programParameter
    : UNUSED IDENTIFIER
    | IDENTIFIER (':=' expression)?
    ;

functionParameters
    : '(' functionParameterList? ')'
    ;

functionParameterList
    : functionParameter (',' functionParameter)*
    ;

functionParameter
    : BYREF? UNUSED? IDENTIFIER (':=' expression)?
    ;

// EXPRESSIONS

scopedFunctionCall
    : IDENTIFIER '::' functionCall
    ;

functionReference
    : '@' IDENTIFIER
    ;

expression
    : primary
    | expression expressionSuffix
    | expression postfix=('++' | '--')
    | prefix=('+'|'-'|'++'|'--') expression
    | prefix=('~'|'!'|'not') expression
    | expression bop=('*' | '/' | '%' | '<<' | '>>' | '&') expression
    | expression bop=('+' | '-' | '|' | '^') expression
    | expression bop='?:' expression
    | expression bop='in' expression
    | expression bop=('<=' | '>=' | '>' | '<') expression
    | expression bop='=' { notifyErrorListeners("Deprecated '=' found: did you mean '==' or ':='?\n"); } expression
    | expression bop=('==' | '!=' | '<>') expression
    | expression bop=('&&' | 'and') expression
    | expression bop=('||' | 'or') expression
    | <assoc=right> expression bop=('.+' | '.-' | '.?') expression
    | <assoc=right> expression
      bop=( ':=' | '+=' | '-=' | '*=' | '/=' | '%=')
      expression
    ;

primary
    : literal
    | parExpression
    | functionCall
    | scopedFunctionCall
    | IDENTIFIER
    | functionReference
    | explicitArrayInitializer
    | explicitStructInitializer
    | explicitDictInitializer
    | explicitErrorInitializer
    | bareArrayInitializer
    | interpolatedString
    ;

explicitArrayInitializer
    : ARRAY arrayInitializer?
    ;

explicitStructInitializer
    : STRUCT structInitializer?
    ;

explicitDictInitializer
    : DICTIONARY dictInitializer?
    ;

explicitErrorInitializer
    : TOK_ERROR structInitializer?
    ;

bareArrayInitializer
    : LBRACE expressionList? RBRACE
    | LBRACE expressionList? ',' RBRACE {notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");}
    ;

parExpression
    : '(' expression ')'
    ;

expressionList
    : expression (',' expression)*
    ;

expressionSuffix
    : indexingSuffix
    | methodCallSuffix
    | navigationSuffix
    ;

indexingSuffix
    : LBRACK expressionList RBRACK
    ;

navigationSuffix
    : '.' ( IDENTIFIER | STRING_LITERAL )
    ;

methodCallSuffix
    : '.' IDENTIFIER LPAREN expressionList? RPAREN
    ;

functionCall
    : IDENTIFIER '(' expressionList? ')'
    ;

structInitializerExpression
    : IDENTIFIER (':=' expression)?
    | STRING_LITERAL (':=' expression)?
    ;

structInitializerExpressionList
    : structInitializerExpression (',' structInitializerExpression)*
    ;

structInitializer
    : '{' structInitializerExpressionList? '}'
    | '{' structInitializerExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in struct initializer list");}
    ;

dictInitializerExpression
    : expression ('->' expression)?
    ;

dictInitializerExpressionList
    : dictInitializerExpression (',' dictInitializerExpression)*
    ;

dictInitializer
    : '{' dictInitializerExpressionList? '}'
    | '{' dictInitializerExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");}
    ;

arrayInitializer
    : '{' expressionList? '}'
    | '{' expressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");}
    | '(' expressionList? ')'
    | '(' expressionList? ',' ')' {notifyErrorListeners("Expected expression following comma before right-paren in array initializer list");}
    ;

// Literals

literal
    : integerLiteral
    | floatLiteral
    | CHAR_LITERAL
    | STRING_LITERAL
    ;

interpolatedString
	: INTERPOLATED_STRING_START interpolatedStringPart* DOUBLE_QUOTE_INSIDE
	;

interpolatedStringPart
	: expression
	| DOUBLE_LBRACE_INSIDE
	| REGULAR_CHAR_INSIDE
	| STRING_LITERAL_INSIDE
	;

integerLiteral
    : DECIMAL_LITERAL
    | HEX_LITERAL
    | OCT_LITERAL
    | BINARY_LITERAL
    ;

floatLiteral
    : FLOAT_LITERAL
    | HEX_FLOAT_LITERAL
    ;
