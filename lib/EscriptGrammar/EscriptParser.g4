parser grammar EscriptParser;

options { tokenVocab=EscriptLexer; }

@header
{
}

@parser::members
{
}

unambiguousCompilationUnit
    : unambiguousTopLevelDeclaration* EOF
    ;

compilationUnit
    : topLevelDeclaration* EOF
    ;

moduleUnit
    : moduleDeclarationStatement* EOF
    ;

unambiguousModuleUnit
    : unambiguousModuleDeclarationStatement* EOF
    ;


moduleDeclarationStatement
    : moduleFunctionDeclaration
    | constStatement
    ;

unambiguousModuleDeclarationStatement
    : unambiguousModuleFunctionDeclaration
    | unambiguousConstStatement
    ;

moduleFunctionDeclaration
    : IDENTIFIER '(' moduleFunctionParameterList? ')' ';'
    ;

unambiguousModuleFunctionDeclaration
    : IDENTIFIER '(' unambiguousModuleFunctionParameterList? ')' ';'
    ;

moduleFunctionParameterList
    : moduleFunctionParameter (',' moduleFunctionParameter)*
    ;

unambiguousModuleFunctionParameterList
    : unambiguousModuleFunctionParameter (',' unambiguousModuleFunctionParameter)*
    ;

moduleFunctionParameter
    : IDENTIFIER (':=' expression)?
    ;

unambiguousModuleFunctionParameter
    : IDENTIFIER (':=' unambiguousExpression)?
    ;

topLevelDeclaration
    : useDeclaration
    | includeDeclaration
    | programDeclaration
    | functionDeclaration
    | statement
    ;

unambiguousTopLevelDeclaration
    : useDeclaration
    | includeDeclaration
    | unambiguousProgramDeclaration
    | unambiguousFunctionDeclaration
    | unambiguousStatement
    ;


functionDeclaration
    : EXPORTED? FUNCTION IDENTIFIER functionParameters block ENDFUNCTION
    ;

unambiguousFunctionDeclaration
    : EXPORTED? FUNCTION IDENTIFIER unambiguousFunctionParameters unambiguousBlock ENDFUNCTION
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

unambiguousProgramDeclaration
    : PROGRAM IDENTIFIER unambiguousProgramParameters unambiguousBlock ENDPROGRAM
    ;

// Some ignored / to-be-handled things:
//  - Labels can only come before DO, WHILE, FOR, FOREACH, REPEAT, and CASE statements.
//  - Const expression must be optimizable

unambiguousStatement
    : unambiguousIfStatement
    | unambiguousReturnStatement
    | unambiguousConstStatement
    | unambiguousVarStatement
    | unambiguousDoStatement
    | unambiguousWhileStatement
    | exitStatement
    | breakStatement
    | continueStatement
    | unambiguousForStatement
    | unambiguousForeachStatement
    | unambiguousRepeatStatement
    | unambiguousCaseStatement
    | unambiguousEnumStatement
    | SEMI
    | statementUnambiguousExpression=unambiguousExpression ';'
    ;

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
    | declareStatement
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

unambiguousIfStatement
    : IF parenthesizedExpression THEN?
          unambiguousBlock
          (ELSEIF parenthesizedExpression unambiguousBlock)*
          (ELSE unambiguousBlock)?
          ENDIF
    ;

gotoStatement
    : GOTO IDENTIFIER ';'
    ;

returnStatement
    : RETURN expression? ';'
    ;

unambiguousReturnStatement
    : RETURN unambiguousExpression? ';'
    ;

constStatement
    : TOK_CONST variableDeclaration ';'
    ;

unambiguousConstStatement
    : TOK_CONST unambiguousVariableDeclaration ';'
    ;

varStatement
    : VAR variableDeclarationList ';'
    ;

unambiguousVarStatement
    : VAR unambiguousVariableDeclarationList ';'
    ;

doStatement
    : statementLabel? DO block DOWHILE parExpression ';'
    ;

unambiguousDoStatement
    : statementLabel? DO unambiguousBlock DOWHILE parenthesizedExpression ';'
    ;

whileStatement
    : statementLabel? WHILE parExpression block ENDWHILE
    ;

unambiguousWhileStatement
    : statementLabel? WHILE parenthesizedExpression unambiguousBlock ENDWHILE
    ;

exitStatement
    : EXIT ';'
    ;

declareStatement
    : DECLARE FUNCTION identifierList ';' 
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

unambiguousForStatement
    : statementLabel? FOR unambiguousForGroup ENDFOR
    ;

foreachStatement
    : statementLabel? FOREACH IDENTIFIER TOK_IN expression block ENDFOREACH
    ;

unambiguousForeachStatement
    : statementLabel? FOREACH IDENTIFIER TOK_IN unambiguousExpression unambiguousBlock ENDFOREACH
    ;

repeatStatement
    : statementLabel? REPEAT block UNTIL expression ';'
    ;

unambiguousRepeatStatement
    : statementLabel? REPEAT unambiguousBlock UNTIL unambiguousExpression ';'
    ;

caseStatement
    : statementLabel? CASE '(' expression ')' switchBlockStatementGroup+ ENDCASE
    ;

unambiguousCaseStatement
    : statementLabel? CASE '(' unambiguousExpression ')' unambiguousSwitchBlockStatementGroup+ ENDCASE
    ;

enumStatement
    : ENUM IDENTIFIER enumList ENDENUM
    ;

unambiguousEnumStatement
    : ENUM IDENTIFIER unambiguousEnumList ENDENUM
    ;

block
    : statement*
    ;

unambiguousBlock
    : unambiguousStatement*
    ;

variableDeclarationInitializer
    : ':=' expression
    | ARRAY
    ;
unambiguousVariableDeclarationInitializer
    : ':=' unambiguousExpression
    | '=' unambiguousExpression {notifyErrorListeners("Unexpected token: '='. Did you mean := for assign?");}
    | ARRAY
    ;

enumList
    : enumListEntry (',' enumListEntry)* ','?
    ;

unambiguousEnumList
    : unambiguousEnumListEntry (',' unambiguousEnumListEntry)* ','?
    ;

enumListEntry
    : IDENTIFIER (':=' expression)?
    ;

unambiguousEnumListEntry
    : IDENTIFIER (':=' unambiguousExpression)?
    ;

switchBlockStatementGroup
    : switchLabel+ block
    ;

unambiguousSwitchBlockStatementGroup
    : switchLabel switchLabel* unambiguousBlock
    ;

switchLabel
    : (integerLiteral | IDENTIFIER | STRING_LITERAL) ':'
    | DEFAULT ':'
    ;

forGroup
    : cstyleForStatement 
    | basicForStatement
    ;

unambiguousForGroup
    : unambiguousCstyleForStatement
    | unambiguousBasicForStatement
    ;

basicForStatement
    : IDENTIFIER ':=' expression TO expression block 
    ;

unambiguousBasicForStatement
    : IDENTIFIER ':=' unambiguousExpression TO unambiguousExpression unambiguousBlock
    ;

cstyleForStatement
    : '(' expression ';' expression ';' expression ')' block
    ;

unambiguousCstyleForStatement
    : '(' unambiguousExpression ';' unambiguousExpression ';' unambiguousExpression ')' unambiguousBlock
    ;

identifierList
    : IDENTIFIER (',' identifierList)?
    ;
variableDeclarationList
    : variableDeclaration (',' variableDeclaration)*
    ;

unambiguousVariableDeclarationList
    : unambiguousVariableDeclaration (',' unambiguousVariableDeclaration)*
    ;

variableDeclaration
    : IDENTIFIER variableDeclarationInitializer?
    ;
unambiguousVariableDeclaration
    : IDENTIFIER unambiguousVariableDeclarationInitializer?
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

unambiguousProgramParameters
    : '(' unambiguousProgramParameterList? ')'
    ;

unambiguousProgramParameterList
    : unambiguousProgramParameter (','? unambiguousProgramParameter)*
    ;

unambiguousProgramParameter
    : UNUSED IDENTIFIER
    | IDENTIFIER (':=' unambiguousExpression)?
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

unambiguousFunctionParameters
    : '(' unambiguousFunctionParameterList? ')'
    ;

unambiguousFunctionParameterList
    : unambiguousFunctionParameter (',' unambiguousFunctionParameter)*
    ;

unambiguousFunctionParameter
    : BYREF? UNUSED? IDENTIFIER (':=' unambiguousExpression)?
    ;

// EXPRESSIONS

scopedMethodCall
    : IDENTIFIER '::' methodCall
    ;

unambiguousExpression
    : membership (assignmentOperator membership)*
    ;

assignmentOperator
    : ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    ;

membership
    : disjunction (membershipOperator disjunction)*
    ;

membershipOperator
    : ADDMEMBER
    | DELMEMBER
    | CHKMEMBER
    ;

disjunction
    : conjunction (disjunctionOperator conjunction)*
    ;

disjunctionOperator
    : OR_A
    | OR_B
    ;

conjunction
    : equality (conjunctionOperator equality)*
    ;

conjunctionOperator
    : AND_A
    | AND_B
    ;

equality
    : bitwiseDisjunction (equalityOperator bitwiseDisjunction)*
    ;

equalityOperator
    : EQUAL
    | EQUAL_DEPRECATED
    | NOTEQUAL_A
    | NOTEQUAL_B
    ;

bitwiseDisjunction
    : bitwiseXor (BITOR bitwiseXor)*
    ;

bitwiseXor
    : bitwiseConjunction (CARET bitwiseConjunction)*
    ;

bitwiseConjunction
    : bitshiftRight (BITAND bitshiftRight)*
    ;

bitshiftRight
    : bitshiftLeft (RSHIFT bitshiftLeft)*
    ;

bitshiftLeft
    : comparison (LSHIFT comparison)*
    ;

comparison
    : infixOperation (comparisonOperator infixOperation)*
    ;

comparisonOperator
    : LE
    | LT
    | GE
    | GT
    ;

infixOperation
    : elvisExpression (infixOperator elvisExpression)*
    ;

infixOperator
    : TOK_IN
    ;

elvisExpression
    : additiveExpression (ELVIS additiveExpression)*
    ;

additiveExpression
    : multiplicativeExpression (additiveOperator multiplicativeExpression)*
    ;

additiveOperator
    : ADD
    | SUB
    ;

multiplicativeExpression
    : prefixUnaryInversionExpression (multiplicativeOperator prefixUnaryInversionExpression)*
    ;

multiplicativeOperator
    : MUL
    | DIV
    | MOD
    ;

prefixUnaryInversionExpression
    : prefixUnaryInversionOperator* prefixUnaryArithmeticExpression
    ;

prefixUnaryInversionOperator
    : TILDE
    | BANG_A
    | BANG_B
    ;

prefixUnaryArithmeticExpression
    : prefixUnaryArithmeticOperator* postfixUnaryExpression
    ;

prefixUnaryArithmeticOperator
    : INC
    | DEC
    | ADD
    | SUB
    ;

postfixUnaryExpression
    : atomicExpression postfixUnaryOperator*
    ;

postfixUnaryOperator
    : INC
    | DEC
    | indexingSuffix
    | navigationSuffix
    | memberCallSuffix
    ;

indexingSuffix
    : LBRACK unambiguousExpressionList RBRACK
    ;

navigationSuffix
    : memberAccessOperator ( IDENTIFIER | STRING_LITERAL )
    ;

membershipSuffix
    : membershipOperator (IDENTIFIER | STRING_LITERAL | unambiguousExpression)
    ;

memberCallSuffix
    : memberAccessOperator IDENTIFIER LPAREN unambiguousExpressionList? RPAREN
    ;

memberAccessOperator
    : DOT
    ;

callSuffix
    : valueArguments
    ;

atomicExpression
    : literal
    | parenthesizedExpression
    | unambiguousFunctionCall
    | IDENTIFIER
    | functionReference
    | unambiguousScopedFunctionCall
    | unambiguousExplicitArrayInitializer
    | unambiguousExplicitStructInitializer
    | unambiguousExplicitDictInitializer
    | unambiguousExplicitErrorInitializer
    | unambiguousBareArrayInitializer
    ;

functionReference
    : '@' IDENTIFIER
    ;

unambiguousExplicitArrayInitializer
    : ARRAY unambiguousArrayInitializer?
    ;

unambiguousExplicitStructInitializer
    : STRUCT unambiguousStructInitializer?
    ;

unambiguousExplicitDictInitializer
    : DICTIONARY unambiguousDictInitializer?
    ;

unambiguousExplicitErrorInitializer
    : TOK_ERROR unambiguousStructInitializer?
    ;

unambiguousBareArrayInitializer
    : LBRACE unambiguousExpressionList? RBRACE
    | LBRACE unambiguousExpressionList? ',' RBRACE {notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");}
    ;

parenthesizedExpression
    : LPAREN unambiguousExpression RPAREN
    ;

expression
    : primary
    | expression bop='.'
      ( IDENTIFIER
      | STRING_LITERAL
      | memberCall
      )
    | expression '[' expressionList ']'
    | methodCall
    | scopedMethodCall
    | ARRAY arrayInitializer?
    | STRUCT structInitializer?
    | DICTIONARY dictInitializer?
    | TOK_ERROR structInitializer?
    | '{' expressionList? '}'
    | functionReference
    | expression postfix=('++' | '--')
    | prefix=('+'|'-'|'++'|'--') expression
    | prefix=('~'|'!'|'not') expression
    | expression bop=('*'|'/'|'%') expression
    | expression bop=('+'|'-') expression
    | expression bop=('<<' | '>>') expression
    | expression bop='?:' expression
    | expression bop=('<=' | '>=' | '>' | '<') expression
    | expression bop=('==' | '=' | '!=' | '<>') expression
    | expression bop='&' expression
    | expression bop='^' expression
    | expression bop='|' expression
    | expression bop='in' expression
    | expression bop=('&&' | 'and') expression
    | expression bop=('||' | 'or') expression
    | <assoc=right> expression bop=('.+' | '.-' | '.?') expression
    | <assoc=right> expression
      bop=( ':=' | '+=' | '-=' | '*=' | '/=' | '%=')
      expression
    ;

primary
    : '(' expression ')'
    | literal
    | IDENTIFIER
    ;

parExpression
    : '(' expression ')'
    ;

expressionList
    : expression (',' expression)*
    ;

unambiguousExpressionList
    : unambiguousExpression (',' unambiguousExpression)*
    ;

methodCallArgument
    : (parameter=IDENTIFIER ':=')? expression
    ;

methodCallArgumentList
    : methodCallArgument (',' methodCallArgument)*
    ;

methodCall
    : IDENTIFIER '(' methodCallArgumentList? ')'
    ;

valueArgumentList
    : unambiguousFunctionCallArgument (',' unambiguousFunctionCallArgument)*
    ;

valueArguments
    : '(' unambiguousFunctionCallArgumentList ')'
    | '(' ')'
    ;

// We treat IDENTIFIER := (expression) differently, but all other expressions
// are valid, for example print( a += 1 );
unambiguousFunctionCallArgument
    : unambiguousExpression
    ;

unambiguousFunctionCall
    : IDENTIFIER valueArguments
    ;

unambiguousScopedFunctionCall
    : IDENTIFIER '::' unambiguousFunctionCall
    ;

unambiguousFunctionCallArgumentList
    : unambiguousFunctionCallArgument (',' unambiguousFunctionCallArgument)*
    ;

memberCall
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
    ;

unambiguousStructInitializerExpression
    : IDENTIFIER (':=' unambiguousExpression)?
    | STRING_LITERAL (':=' unambiguousExpression)?
    ;

unambiguousStructInitializerExpressionList
    : unambiguousStructInitializerExpression (',' unambiguousStructInitializerExpression)*
    ;

unambiguousStructInitializer
    : '{' unambiguousStructInitializerExpressionList? '}'
    | '{' unambiguousStructInitializerExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in struct initializer list");}
    ;

dictInitializerExpression
    : expression ('->' expression)?
    ;

unambiguousDictInitializerExpression
    : unambiguousExpression ('->' unambiguousExpression)?
    ;

dictInitializerExpressionList
    : dictInitializerExpression (',' dictInitializerExpression)*
    ;

unambiguousDictInitializerExpressionList
    : unambiguousDictInitializerExpression (',' unambiguousDictInitializerExpression)*
    ;

dictInitializer
    : '{' dictInitializerExpressionList? '}'
    | '{' dictInitializerExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");}
    ;

unambiguousDictInitializer
    : '{' unambiguousDictInitializerExpressionList? '}'
    | '{' unambiguousDictInitializerExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");}
    ;

arrayInitializer
    : '{' expressionList? '}'
    | '(' expressionList? ')'
    ;

unambiguousArrayInitializer
    : '{' unambiguousExpressionList? '}'
    | '(' unambiguousExpressionList? ')'
    | '{' unambiguousExpressionList? ',' '}' {notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");}
    | '(' unambiguousExpressionList? ',' ')' {notifyErrorListeners("Expected expression following comma before right-paren in array initializer list");}
    ;

// Literals

literal
    : integerLiteral
    | floatLiteral
    | CHAR_LITERAL
    | STRING_LITERAL
    | NULL_LITERAL
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
