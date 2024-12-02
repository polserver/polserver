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

evaluateUnit
    : expression EOF
    ;

moduleDeclarationStatement
    : moduleFunctionDeclaration
    | constStatement
    ;

moduleFunctionDeclaration
    : IDENTIFIER '(' moduleFunctionParameterList? ')' (ARROW typeArgument)? ';'
    ;

moduleFunctionParameterList
    : moduleFunctionParameter (',' moduleFunctionParameter)*
    ;

moduleFunctionParameter
    : IDENTIFIER typeAnnotation? (':=' expression)?
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
    ;

functionDeclaration
    : EXPORTED? FUNCTION IDENTIFIER typeParameters? functionParameters (ARROW typeArgument)? block ENDFUNCTION
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
    : PROGRAM IDENTIFIER typeParameters? programParameters (ARROW typeArgument)? block ENDPROGRAM
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
    : (integerLiteral | boolLiteral | UNINIT | IDENTIFIER | STRING_LITERAL) ':'
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
    : IDENTIFIER typeAnnotation? variableDeclarationInitializer
    ;

variableDeclaration
    : IDENTIFIER typeAnnotation? variableDeclarationInitializer?
    ;

// PARAMETERS
programParameters
    : '(' programParameterList? ')'
    ;

programParameterList
    : programParameter (','? programParameter)*
    ;

programParameter
    : UNUSED IDENTIFIER typeAnnotation?
    | IDENTIFIER typeAnnotation? (':=' expression)?
    ;

functionParameters
    : '(' functionParameterList? ')'
    ;

functionParameterList
    : functionParameter (',' functionParameter)*
    ;

functionParameter
    : BYREF? UNUSED? IDENTIFIER typeAnnotation? ELLIPSIS? (':=' expression)?
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
    | expression bop=('*' | '/' | '%' | '<<') expression
    | expression '>' { if ( _input->get( _input->index() -1 )->getType() != GT ) notifyErrorListeners( "Expression expected" ); } '>'  expression
    | expression bop='&' expression
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
    | expression AS typeArgument
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
    : AT functionParameters? LBRACE block RBRACE
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

// Experimental type support
typeParameters
    : '<' typeParameterList '>'
    ;

typeParameterList
    : typeParameter (',' typeParameter)*
    ;

typeParameter
    : IDENTIFIER constraint?
    | IDENTIFIER ':=' typeArgument
    | typeParameters
    ;

constraint
    : IDENTIFIER type
    ;

typeArguments
    : '<' typeArgumentList? '>'
    ;

typeArgumentList
    : typeArgument (',' typeArgument)*
    ;

typeArgument
    : type
    ;

type
    : binaryOrPrimaryType
    | functionType
    ;

binaryOrPrimaryType
    : binaryOrPrimaryType ('|' | '&') binaryOrPrimaryType
    | primaryType
    ;

primaryType
    : '(' type ')'
    | predefinedType
    | objectType
    | typeReference
    | primaryType {/*todo this.notLineTerminator()*/ true}? '[' ']'
    | '[' tupleElementTypes ']'
    ;

predefinedType
    : INTEGER
    | DOUBLE
    | STRING
    | UNINIT
    | ARRAY
    | TOK_LONG  // Alias of INTEGER
    ;

typeReference
    : identifierName typeGeneric?
    ;

typeGeneric
    : '<' typeArgumentList typeGeneric?'>'
    ;

objectType
    : (STRUCT | DICTIONARY) ('{' typeBody? '}')?
    ;

typeBody
    : typeMemberList (';' | ',')?
    ;

typeMemberList
    : typeMember ((';' | ',') typeMember)*
    ;

typeMember
    : propertySignature
    | callSignature
    | indexSignature
    | methodSignature (ARROW type)?
    ;

// Tuples can have a trailing comma.
tupleElementTypes
    : type (',' type)* ','?
    ;

functionType
    : typeParameters? '(' parameterList? ')' ARROW type
    ;

identifierName
    : IDENTIFIER
    | reservedWord
    ;

reservedWord
    : IF
    | THEN
    | ELSEIF
    | ENDIF
    | ELSE
    | GOTO
    | RETURN
    | TOK_CONST
    | VAR
    | DO
    | DOWHILE
    | WHILE
    | ENDWHILE
    | EXIT
    | FUNCTION
    | ENDFUNCTION
    | EXPORTED
    | USE
    | INCLUDE
    | BREAK
    | CONTINUE
    | FOR
    | ENDFOR
    | TO
    | FOREACH
    | ENDFOREACH
    | REPEAT
    | UNTIL
    | PROGRAM
    | ENDPROGRAM
    | CASE
    | DEFAULT
    | ENDCASE
    | ENUM
    | ENDENUM
    | CLASS
    | ENDCLASS
    | DOWNTO
    | STEP
    | REFERENCE
    | TOK_OUT
    | INOUT
    | BYVAL
    | STRING
    | TOK_LONG
    | INTEGER
    | UNSIGNED
    | SIGNED
    | REAL
    | FLOAT
    | DOUBLE
    | AS
    | AND_B
    | OR_B
    | BANG_B
    | BYREF
    | UNUSED
    | TOK_ERROR
    | HASH
    | DICTIONARY
    | STRUCT
    | ARRAY
    | STACK
    | TOK_IN
    | UNINIT
    | BOOL_TRUE
    | BOOL_FALSE
    | IS
    ;

propertySignature
    : propertyName '?'? typeAnnotation?
    ;

propertyName
    : IDENTIFIER
    | reservedWord
    ;

typeAnnotation
    : ':' type
    ;

callSignature
    : typeParameters? '(' parameterList? ')' typeAnnotation?
    ;

// Function parameter list can have a trailing comma.
parameterList
    : restParameter
    | parameter (',' parameter)* (',' restParameter)? ','?
    ;

parameter
    : IDENTIFIER '?:' type
    | IDENTIFIER '?'? ':' type
    ;

restParameter
    : IDENTIFIER '...' typeAnnotation?
    ;

indexSignature
    : '[' parameterList ']' typeAnnotation
    ;

methodSignature
    : propertyName '?'? callSignature
    ;
