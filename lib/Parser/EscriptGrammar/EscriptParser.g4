parser grammar EscriptParser;

options { tokenVocab=EscriptLexer; }

@header
{
}

@parser::members
{
    bool inUninitFunction = false;
}


compilationUnit
    : topLevelDeclaration* EOF
    ;

moduleUnit
    : moduleDeclarationStatement* EOF
    ;

evaluateUnit
    : expression EOF
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
    | classDeclaration
    | statement
    ;

classDeclaration
    : CLASS IDENTIFIER classParameters classBody ENDCLASS
    ;

classParameters
    : '(' classParameterList? ')'
    ;

classParameterList
    : IDENTIFIER (',' IDENTIFIER)*
    ;

classBody
    : classStatement*
    ;

classStatement
    : functionDeclaration
    | varStatement
    | uninitFunctionDeclaration
    ;

uninitFunctionDeclaration
    : UNINIT FUNCTION IDENTIFIER { inUninitFunction = true; } functionParameters ';'
    ;

functionDeclaration
    : EXPORTED? FUNCTION IDENTIFIER { inUninitFunction = false; } functionParameters block ENDFUNCTION
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
    : TOK_CONST constantDeclaration ';'
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
    | scopedIdentifier
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
    : ENUM CLASS? IDENTIFIER enumList ENDENUM
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
    : (integerLiteral | boolLiteral | scopedIdentifier | UNINIT | IDENTIFIER | STRING_LITERAL) ':'
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

constantDeclaration
    : IDENTIFIER variableDeclarationInitializer
    ;

variableDeclaration
    : IDENTIFIER variableDeclarationInitializer?
    | bindingDeclaration bindingDeclarationInitializer
    ;

bindingDeclaration
    :  LBRACK sequenceBindingList RBRACK
    |  LBRACE indexBindingList RBRACE
    ;

indexBindingList
    : indexBinding (',' indexBinding)*
    ;

sequenceBindingList
    : sequenceBinding (',' sequenceBinding)*
    ;

sequenceBinding
    : IDENTIFIER ELLIPSIS?
    | bindingDeclaration
    ;

indexBinding
    : IDENTIFIER ELLIPSIS?
    | IDENTIFIER binding?
    | LBRACK expression RBRACK binding
    ;

binding
    : ':' IDENTIFIER
    | ':' bindingDeclaration
    ;

bindingDeclarationInitializer
    : ':=' expression
    | '=' expression { notifyErrorListeners("Unexpected token: '='. Did you mean := for assign?\n"); }
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
    : BYREF?
      ( { if ( inUninitFunction ) notifyErrorListeners( "The 'unused' keyword may not be used here." ); } UNUSED)?
      ( { if ( !inUninitFunction ) notifyErrorListeners( "The 'default' keyword may not be used here." ); } DEFAULT)?
      IDENTIFIER
      ELLIPSIS?
      ( ':=' { if ( inUninitFunction ) notifyErrorListeners( "A parameter may not be given a default value here." ); } expression )?
    ;

// EXPRESSIONS
//
// Currently no module functions return a function reference, so no need to
// support eg. `uo::Foo( bar )( baz )`.
scopedFunctionCall
    : IDENTIFIER? '::' functionCall
    ;

functionReference
    : '@' (scope=IDENTIFIER? '::')? function=IDENTIFIER
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
    | expression bop='is' expression
    | expression bop='=' { notifyErrorListeners("Deprecated '=' found: did you mean '==' or ':='?\n"); } expression
    | expression bop=('==' | '!=' | '<>') expression
    | expression bop=('&&' | 'and') expression
    | expression bop=('||' | 'or') expression
    | expression '?' expression ':' expression
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
    | scopedIdentifier
    | IDENTIFIER
    | functionReference
    | functionExpression
    | explicitArrayInitializer
    | explicitStructInitializer
    | explicitDictInitializer
    | explicitErrorInitializer
    | bareArrayInitializer
    | interpolatedString
    ;

scopedIdentifier
    : scope=IDENTIFIER? '::' identifier=IDENTIFIER;

functionExpression
    : AT { inUninitFunction = false; } functionParameters? LBRACE block RBRACE
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
    : expressionListEntry (',' expressionListEntry)*
    ;

expressionListEntry
    : expression ELLIPSIS?
    ;

expressionSuffix
    : indexingSuffix
    | methodCallSuffix
    | navigationSuffix
    | functionCallSuffix
    ;

indexingSuffix
    : LBRACK indexList RBRACK
    ;

indexList
    : expression (',' expression)*
    ;

navigationSuffix
    : '.' ( IDENTIFIER | STRING_LITERAL | FUNCTION )
    ;

methodCallSuffix
    : '.' IDENTIFIER LPAREN expressionList? RPAREN
    ;

functionCallSuffix
    : '(' expressionList? ')'
    ;

functionCall
    : IDENTIFIER '(' expressionList? ')'
    ;

structInitializerExpression
    : IDENTIFIER (':=' expression)?
    | STRING_LITERAL (':=' expression)?
    | expression ELLIPSIS
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
    | expression ELLIPSIS
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
    | boolLiteral
    | STRING_LITERAL
    | UNINIT
    ;

interpolatedString
	: INTERPOLATED_STRING_START interpolatedStringPart* DOUBLE_QUOTE_INSIDE
	;

interpolatedStringPart
	: LBRACE_INSIDE expression (':' FORMAT_STRING)?
	| LBRACE_INSIDE (':' FORMAT_STRING)? {notifyErrorListeners("Expected expression following interpolated string part start");}
	| DOUBLE_LBRACE_INSIDE
	| REGULAR_CHAR_INSIDE
    | DOUBLE_RBRACE
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

boolLiteral
    : BOOL_TRUE
    | BOOL_FALSE
    ;
