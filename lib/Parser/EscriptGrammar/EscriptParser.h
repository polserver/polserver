


// Generated from EscriptParser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"


namespace EscriptGrammar {


class  EscriptParser : public antlr4::Parser {
public:
  enum {
    IF = 1, THEN = 2, ELSEIF = 3, ENDIF = 4, ELSE = 5, GOTO = 6, RETURN = 7, 
    TOK_CONST = 8, VAR = 9, DO = 10, DOWHILE = 11, WHILE = 12, ENDWHILE = 13, 
    EXIT = 14, FUNCTION = 15, ENDFUNCTION = 16, EXPORTED = 17, USE = 18, 
    INCLUDE = 19, BREAK = 20, CONTINUE = 21, FOR = 22, ENDFOR = 23, TO = 24, 
    FOREACH = 25, ENDFOREACH = 26, REPEAT = 27, UNTIL = 28, PROGRAM = 29, 
    ENDPROGRAM = 30, CASE = 31, DEFAULT = 32, ENDCASE = 33, ENUM = 34, ENDENUM = 35, 
    CLASS = 36, ENDCLASS = 37, DOWNTO = 38, STEP = 39, REFERENCE = 40, TOK_OUT = 41, 
    INOUT = 42, BYVAL = 43, STRING = 44, TOK_LONG = 45, INTEGER = 46, UNSIGNED = 47, 
    SIGNED = 48, REAL = 49, FLOAT = 50, DOUBLE = 51, AS = 52, ELLIPSIS = 53, 
    AND_A = 54, AND_B = 55, OR_A = 56, OR_B = 57, BANG_A = 58, BANG_B = 59, 
    BYREF = 60, UNUSED = 61, TOK_ERROR = 62, HASH = 63, DICTIONARY = 64, 
    STRUCT = 65, ARRAY = 66, STACK = 67, TOK_IN = 68, UNINIT = 69, BOOL_TRUE = 70, 
    BOOL_FALSE = 71, IS = 72, DECIMAL_LITERAL = 73, HEX_LITERAL = 74, OCT_LITERAL = 75, 
    BINARY_LITERAL = 76, FLOAT_LITERAL = 77, HEX_FLOAT_LITERAL = 78, STRING_LITERAL = 79, 
    INTERPOLATED_STRING_START = 80, LPAREN = 81, RPAREN = 82, LBRACK = 83, 
    RBRACK = 84, LBRACE = 85, RBRACE = 86, DOT = 87, ARROW = 88, MUL = 89, 
    DIV = 90, MOD = 91, ADD = 92, SUB = 93, ADD_ASSIGN = 94, SUB_ASSIGN = 95, 
    MUL_ASSIGN = 96, DIV_ASSIGN = 97, MOD_ASSIGN = 98, LE = 99, LT = 100, 
    GE = 101, GT = 102, LSHIFT = 103, BITAND = 104, CARET = 105, BITOR = 106, 
    NOTEQUAL_A = 107, NOTEQUAL_B = 108, EQUAL_DEPRECATED = 109, EQUAL = 110, 
    ASSIGN = 111, ADDMEMBER = 112, DELMEMBER = 113, CHKMEMBER = 114, SEMI = 115, 
    COMMA = 116, TILDE = 117, AT = 118, COLONCOLON = 119, COLON = 120, INC = 121, 
    DEC = 122, ELVIS = 123, QUESTION = 124, WS = 125, COMMENT = 126, LINE_COMMENT = 127, 
    IDENTIFIER = 128, DOUBLE_LBRACE_INSIDE = 129, LBRACE_INSIDE = 130, REGULAR_CHAR_INSIDE = 131, 
    DOUBLE_QUOTE_INSIDE = 132, DOUBLE_RBRACE = 133, STRING_LITERAL_INSIDE = 134, 
    CLOSE_RBRACE_INSIDE = 135, FORMAT_STRING = 136
  };

  enum {
    RuleCompilationUnit = 0, RuleModuleUnit = 1, RuleEvaluateUnit = 2, RuleModuleDeclarationStatement = 3, 
    RuleModuleFunctionDeclaration = 4, RuleModuleFunctionParameterList = 5, 
    RuleModuleFunctionParameter = 6, RuleTopLevelDeclaration = 7, RuleClassDeclaration = 8, 
    RuleClassParameters = 9, RuleClassParameterList = 10, RuleClassBody = 11, 
    RuleClassStatement = 12, RuleFunctionDeclaration = 13, RuleStringIdentifier = 14, 
    RuleUseDeclaration = 15, RuleIncludeDeclaration = 16, RuleProgramDeclaration = 17, 
    RuleStatement = 18, RuleStatementLabel = 19, RuleIfStatement = 20, RuleGotoStatement = 21, 
    RuleReturnStatement = 22, RuleConstStatement = 23, RuleVarStatement = 24, 
    RuleDoStatement = 25, RuleWhileStatement = 26, RuleExitStatement = 27, 
    RuleBreakStatement = 28, RuleContinueStatement = 29, RuleForStatement = 30, 
    RuleForeachIterableExpression = 31, RuleForeachStatement = 32, RuleRepeatStatement = 33, 
    RuleCaseStatement = 34, RuleEnumStatement = 35, RuleBlock = 36, RuleVariableDeclarationInitializer = 37, 
    RuleEnumList = 38, RuleEnumListEntry = 39, RuleSwitchBlockStatementGroup = 40, 
    RuleSwitchLabel = 41, RuleForGroup = 42, RuleBasicForStatement = 43, 
    RuleCstyleForStatement = 44, RuleIdentifierList = 45, RuleVariableDeclarationList = 46, 
    RuleConstantDeclaration = 47, RuleVariableDeclaration = 48, RuleProgramParameters = 49, 
    RuleProgramParameterList = 50, RuleProgramParameter = 51, RuleFunctionParameters = 52, 
    RuleFunctionParameterList = 53, RuleFunctionParameter = 54, RuleScopedFunctionCall = 55, 
    RuleFunctionReference = 56, RuleExpression = 57, RulePrimary = 58, RuleScopedIdentifier = 59, 
    RuleFunctionExpression = 60, RuleExplicitArrayInitializer = 61, RuleExplicitStructInitializer = 62, 
    RuleExplicitDictInitializer = 63, RuleExplicitErrorInitializer = 64, 
    RuleBareArrayInitializer = 65, RuleParExpression = 66, RuleExpressionList = 67, 
    RuleExpressionListEntry = 68, RuleExpressionSuffix = 69, RuleIndexingSuffix = 70, 
    RuleIndexList = 71, RuleNavigationSuffix = 72, RuleMethodCallSuffix = 73, 
    RuleFunctionCallSuffix = 74, RuleFunctionCall = 75, RuleStructInitializerExpression = 76, 
    RuleStructInitializerExpressionList = 77, RuleStructInitializer = 78, 
    RuleDictInitializerExpression = 79, RuleDictInitializerExpressionList = 80, 
    RuleDictInitializer = 81, RuleArrayInitializer = 82, RuleLiteral = 83, 
    RuleInterpolatedString = 84, RuleInterpolatedStringPart = 85, RuleIntegerLiteral = 86, 
    RuleFloatLiteral = 87, RuleBoolLiteral = 88, RuleTypeParameters = 89, 
    RuleTypeParameterList = 90, RuleTypeParameter = 91, RuleConstraint = 92, 
    RuleTypeArguments = 93, RuleTypeArgumentList = 94, RuleTypeArgument = 95, 
    RuleType = 96, RuleBinaryOrPrimaryType = 97, RulePrimaryType = 98, RulePredefinedType = 99, 
    RuleTypeReference = 100, RuleTypeGeneric = 101, RuleObjectType = 102, 
    RuleTypeBody = 103, RuleTypeMemberList = 104, RuleTypeMember = 105, 
    RuleTupleElementTypes = 106, RuleFunctionType = 107, RuleIdentifierName = 108, 
    RuleReservedWord = 109, RulePropertySignature = 110, RulePropertyName = 111, 
    RuleTypeAnnotation = 112, RuleCallSignature = 113, RuleParameterList = 114, 
    RuleParameter = 115, RuleRestParameter = 116, RuleIndexSignature = 117, 
    RuleMethodSignature = 118
  };

  explicit EscriptParser(antlr4::TokenStream *input);

  EscriptParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~EscriptParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;




  class CompilationUnitContext;
  class ModuleUnitContext;
  class EvaluateUnitContext;
  class ModuleDeclarationStatementContext;
  class ModuleFunctionDeclarationContext;
  class ModuleFunctionParameterListContext;
  class ModuleFunctionParameterContext;
  class TopLevelDeclarationContext;
  class ClassDeclarationContext;
  class ClassParametersContext;
  class ClassParameterListContext;
  class ClassBodyContext;
  class ClassStatementContext;
  class FunctionDeclarationContext;
  class StringIdentifierContext;
  class UseDeclarationContext;
  class IncludeDeclarationContext;
  class ProgramDeclarationContext;
  class StatementContext;
  class StatementLabelContext;
  class IfStatementContext;
  class GotoStatementContext;
  class ReturnStatementContext;
  class ConstStatementContext;
  class VarStatementContext;
  class DoStatementContext;
  class WhileStatementContext;
  class ExitStatementContext;
  class BreakStatementContext;
  class ContinueStatementContext;
  class ForStatementContext;
  class ForeachIterableExpressionContext;
  class ForeachStatementContext;
  class RepeatStatementContext;
  class CaseStatementContext;
  class EnumStatementContext;
  class BlockContext;
  class VariableDeclarationInitializerContext;
  class EnumListContext;
  class EnumListEntryContext;
  class SwitchBlockStatementGroupContext;
  class SwitchLabelContext;
  class ForGroupContext;
  class BasicForStatementContext;
  class CstyleForStatementContext;
  class IdentifierListContext;
  class VariableDeclarationListContext;
  class ConstantDeclarationContext;
  class VariableDeclarationContext;
  class ProgramParametersContext;
  class ProgramParameterListContext;
  class ProgramParameterContext;
  class FunctionParametersContext;
  class FunctionParameterListContext;
  class FunctionParameterContext;
  class ScopedFunctionCallContext;
  class FunctionReferenceContext;
  class ExpressionContext;
  class PrimaryContext;
  class ScopedIdentifierContext;
  class FunctionExpressionContext;
  class ExplicitArrayInitializerContext;
  class ExplicitStructInitializerContext;
  class ExplicitDictInitializerContext;
  class ExplicitErrorInitializerContext;
  class BareArrayInitializerContext;
  class ParExpressionContext;
  class ExpressionListContext;
  class ExpressionListEntryContext;
  class ExpressionSuffixContext;
  class IndexingSuffixContext;
  class IndexListContext;
  class NavigationSuffixContext;
  class MethodCallSuffixContext;
  class FunctionCallSuffixContext;
  class FunctionCallContext;
  class StructInitializerExpressionContext;
  class StructInitializerExpressionListContext;
  class StructInitializerContext;
  class DictInitializerExpressionContext;
  class DictInitializerExpressionListContext;
  class DictInitializerContext;
  class ArrayInitializerContext;
  class LiteralContext;
  class InterpolatedStringContext;
  class InterpolatedStringPartContext;
  class IntegerLiteralContext;
  class FloatLiteralContext;
  class BoolLiteralContext;
  class TypeParametersContext;
  class TypeParameterListContext;
  class TypeParameterContext;
  class ConstraintContext;
  class TypeArgumentsContext;
  class TypeArgumentListContext;
  class TypeArgumentContext;
  class TypeContext;
  class BinaryOrPrimaryTypeContext;
  class PrimaryTypeContext;
  class PredefinedTypeContext;
  class TypeReferenceContext;
  class TypeGenericContext;
  class ObjectTypeContext;
  class TypeBodyContext;
  class TypeMemberListContext;
  class TypeMemberContext;
  class TupleElementTypesContext;
  class FunctionTypeContext;
  class IdentifierNameContext;
  class ReservedWordContext;
  class PropertySignatureContext;
  class PropertyNameContext;
  class TypeAnnotationContext;
  class CallSignatureContext;
  class ParameterListContext;
  class ParameterContext;
  class RestParameterContext;
  class IndexSignatureContext;
  class MethodSignatureContext; 

  class  CompilationUnitContext : public antlr4::ParserRuleContext {
  public:
    CompilationUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<TopLevelDeclarationContext *> topLevelDeclaration();
    TopLevelDeclarationContext* topLevelDeclaration(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CompilationUnitContext* compilationUnit();

  class  ModuleUnitContext : public antlr4::ParserRuleContext {
  public:
    ModuleUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<ModuleDeclarationStatementContext *> moduleDeclarationStatement();
    ModuleDeclarationStatementContext* moduleDeclarationStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleUnitContext* moduleUnit();

  class  EvaluateUnitContext : public antlr4::ParserRuleContext {
  public:
    EvaluateUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EvaluateUnitContext* evaluateUnit();

  class  ModuleDeclarationStatementContext : public antlr4::ParserRuleContext {
  public:
    ModuleDeclarationStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ModuleFunctionDeclarationContext *moduleFunctionDeclaration();
    ConstStatementContext *constStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleDeclarationStatementContext* moduleDeclarationStatement();

  class  ModuleFunctionDeclarationContext : public antlr4::ParserRuleContext {
  public:
    ModuleFunctionDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *SEMI();
    ModuleFunctionParameterListContext *moduleFunctionParameterList();
    antlr4::tree::TerminalNode *ARROW();
    TypeArgumentContext *typeArgument();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionDeclarationContext* moduleFunctionDeclaration();

  class  ModuleFunctionParameterListContext : public antlr4::ParserRuleContext {
  public:
    ModuleFunctionParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ModuleFunctionParameterContext *> moduleFunctionParameter();
    ModuleFunctionParameterContext* moduleFunctionParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionParameterListContext* moduleFunctionParameterList();

  class  ModuleFunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    ModuleFunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    TypeAnnotationContext *typeAnnotation();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionParameterContext* moduleFunctionParameter();

  class  TopLevelDeclarationContext : public antlr4::ParserRuleContext {
  public:
    TopLevelDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UseDeclarationContext *useDeclaration();
    IncludeDeclarationContext *includeDeclaration();
    ProgramDeclarationContext *programDeclaration();
    FunctionDeclarationContext *functionDeclaration();
    ClassDeclarationContext *classDeclaration();
    StatementContext *statement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TopLevelDeclarationContext* topLevelDeclaration();

  class  ClassDeclarationContext : public antlr4::ParserRuleContext {
  public:
    ClassDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CLASS();
    antlr4::tree::TerminalNode *IDENTIFIER();
    ClassParametersContext *classParameters();
    ClassBodyContext *classBody();
    antlr4::tree::TerminalNode *ENDCLASS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassDeclarationContext* classDeclaration();

  class  ClassParametersContext : public antlr4::ParserRuleContext {
  public:
    ClassParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ClassParameterListContext *classParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassParametersContext* classParameters();

  class  ClassParameterListContext : public antlr4::ParserRuleContext {
  public:
    ClassParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassParameterListContext* classParameterList();

  class  ClassBodyContext : public antlr4::ParserRuleContext {
  public:
    ClassBodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ClassStatementContext *> classStatement();
    ClassStatementContext* classStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassBodyContext* classBody();

  class  ClassStatementContext : public antlr4::ParserRuleContext {
  public:
    ClassStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    FunctionDeclarationContext *functionDeclaration();
    VarStatementContext *varStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClassStatementContext* classStatement();

  class  FunctionDeclarationContext : public antlr4::ParserRuleContext {
  public:
    FunctionDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *IDENTIFIER();
    FunctionParametersContext *functionParameters();
    BlockContext *block();
    antlr4::tree::TerminalNode *ENDFUNCTION();
    antlr4::tree::TerminalNode *EXPORTED();
    TypeParametersContext *typeParameters();
    antlr4::tree::TerminalNode *ARROW();
    TypeArgumentContext *typeArgument();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionDeclarationContext* functionDeclaration();

  class  StringIdentifierContext : public antlr4::ParserRuleContext {
  public:
    StringIdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StringIdentifierContext* stringIdentifier();

  class  UseDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UseDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *USE();
    StringIdentifierContext *stringIdentifier();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UseDeclarationContext* useDeclaration();

  class  IncludeDeclarationContext : public antlr4::ParserRuleContext {
  public:
    IncludeDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INCLUDE();
    StringIdentifierContext *stringIdentifier();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IncludeDeclarationContext* includeDeclaration();

  class  ProgramDeclarationContext : public antlr4::ParserRuleContext {
  public:
    ProgramDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PROGRAM();
    antlr4::tree::TerminalNode *IDENTIFIER();
    ProgramParametersContext *programParameters();
    BlockContext *block();
    antlr4::tree::TerminalNode *ENDPROGRAM();
    TypeParametersContext *typeParameters();
    antlr4::tree::TerminalNode *ARROW();
    TypeArgumentContext *typeArgument();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramDeclarationContext* programDeclaration();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    EscriptParser::ExpressionContext *statementExpression = nullptr;
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IfStatementContext *ifStatement();
    GotoStatementContext *gotoStatement();
    ReturnStatementContext *returnStatement();
    ConstStatementContext *constStatement();
    VarStatementContext *varStatement();
    DoStatementContext *doStatement();
    WhileStatementContext *whileStatement();
    ExitStatementContext *exitStatement();
    BreakStatementContext *breakStatement();
    ContinueStatementContext *continueStatement();
    ForStatementContext *forStatement();
    ForeachStatementContext *foreachStatement();
    RepeatStatementContext *repeatStatement();
    CaseStatementContext *caseStatement();
    EnumStatementContext *enumStatement();
    antlr4::tree::TerminalNode *SEMI();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  StatementLabelContext : public antlr4::ParserRuleContext {
  public:
    StatementLabelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COLON();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementLabelContext* statementLabel();

  class  IfStatementContext : public antlr4::ParserRuleContext {
  public:
    IfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    std::vector<ParExpressionContext *> parExpression();
    ParExpressionContext* parExpression(size_t i);
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    antlr4::tree::TerminalNode *ENDIF();
    antlr4::tree::TerminalNode *THEN();
    std::vector<antlr4::tree::TerminalNode *> ELSEIF();
    antlr4::tree::TerminalNode* ELSEIF(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IfStatementContext* ifStatement();

  class  GotoStatementContext : public antlr4::ParserRuleContext {
  public:
    GotoStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GOTO();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GotoStatementContext* gotoStatement();

  class  ReturnStatementContext : public antlr4::ParserRuleContext {
  public:
    ReturnStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURN();
    antlr4::tree::TerminalNode *SEMI();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnStatementContext* returnStatement();

  class  ConstStatementContext : public antlr4::ParserRuleContext {
  public:
    ConstStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_CONST();
    ConstantDeclarationContext *constantDeclaration();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstStatementContext* constStatement();

  class  VarStatementContext : public antlr4::ParserRuleContext {
  public:
    VarStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VAR();
    VariableDeclarationListContext *variableDeclarationList();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarStatementContext* varStatement();

  class  DoStatementContext : public antlr4::ParserRuleContext {
  public:
    DoStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DO();
    BlockContext *block();
    antlr4::tree::TerminalNode *DOWHILE();
    ParExpressionContext *parExpression();
    antlr4::tree::TerminalNode *SEMI();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DoStatementContext* doStatement();

  class  WhileStatementContext : public antlr4::ParserRuleContext {
  public:
    WhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WHILE();
    ParExpressionContext *parExpression();
    BlockContext *block();
    antlr4::tree::TerminalNode *ENDWHILE();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhileStatementContext* whileStatement();

  class  ExitStatementContext : public antlr4::ParserRuleContext {
  public:
    ExitStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EXIT();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExitStatementContext* exitStatement();

  class  BreakStatementContext : public antlr4::ParserRuleContext {
  public:
    BreakStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BREAK();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BreakStatementContext* breakStatement();

  class  ContinueStatementContext : public antlr4::ParserRuleContext {
  public:
    ContinueStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CONTINUE();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ContinueStatementContext* continueStatement();

  class  ForStatementContext : public antlr4::ParserRuleContext {
  public:
    ForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOR();
    ForGroupContext *forGroup();
    antlr4::tree::TerminalNode *ENDFOR();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForStatementContext* forStatement();

  class  ForeachIterableExpressionContext : public antlr4::ParserRuleContext {
  public:
    ForeachIterableExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    FunctionCallContext *functionCall();
    ScopedFunctionCallContext *scopedFunctionCall();
    ScopedIdentifierContext *scopedIdentifier();
    antlr4::tree::TerminalNode *IDENTIFIER();
    ParExpressionContext *parExpression();
    BareArrayInitializerContext *bareArrayInitializer();
    ExplicitArrayInitializerContext *explicitArrayInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForeachIterableExpressionContext* foreachIterableExpression();

  class  ForeachStatementContext : public antlr4::ParserRuleContext {
  public:
    ForeachStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOREACH();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *TOK_IN();
    ForeachIterableExpressionContext *foreachIterableExpression();
    BlockContext *block();
    antlr4::tree::TerminalNode *ENDFOREACH();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForeachStatementContext* foreachStatement();

  class  RepeatStatementContext : public antlr4::ParserRuleContext {
  public:
    RepeatStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REPEAT();
    BlockContext *block();
    antlr4::tree::TerminalNode *UNTIL();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *SEMI();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RepeatStatementContext* repeatStatement();

  class  CaseStatementContext : public antlr4::ParserRuleContext {
  public:
    CaseStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CASE();
    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *ENDCASE();
    StatementLabelContext *statementLabel();
    std::vector<SwitchBlockStatementGroupContext *> switchBlockStatementGroup();
    SwitchBlockStatementGroupContext* switchBlockStatementGroup(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CaseStatementContext* caseStatement();

  class  EnumStatementContext : public antlr4::ParserRuleContext {
  public:
    EnumStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ENUM();
    antlr4::tree::TerminalNode *IDENTIFIER();
    EnumListContext *enumList();
    antlr4::tree::TerminalNode *ENDENUM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumStatementContext* enumStatement();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  VariableDeclarationInitializerContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *EQUAL_DEPRECATED();
    antlr4::tree::TerminalNode *ARRAY();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationInitializerContext* variableDeclarationInitializer();

  class  EnumListContext : public antlr4::ParserRuleContext {
  public:
    EnumListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<EnumListEntryContext *> enumListEntry();
    EnumListEntryContext* enumListEntry(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumListContext* enumList();

  class  EnumListEntryContext : public antlr4::ParserRuleContext {
  public:
    EnumListEntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumListEntryContext* enumListEntry();

  class  SwitchBlockStatementGroupContext : public antlr4::ParserRuleContext {
  public:
    SwitchBlockStatementGroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BlockContext *block();
    std::vector<SwitchLabelContext *> switchLabel();
    SwitchLabelContext* switchLabel(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SwitchBlockStatementGroupContext* switchBlockStatementGroup();

  class  SwitchLabelContext : public antlr4::ParserRuleContext {
  public:
    SwitchLabelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    IntegerLiteralContext *integerLiteral();
    BoolLiteralContext *boolLiteral();
    antlr4::tree::TerminalNode *UNINIT();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *DEFAULT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SwitchLabelContext* switchLabel();

  class  ForGroupContext : public antlr4::ParserRuleContext {
  public:
    ForGroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CstyleForStatementContext *cstyleForStatement();
    BasicForStatementContext *basicForStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForGroupContext* forGroup();

  class  BasicForStatementContext : public antlr4::ParserRuleContext {
  public:
    BasicForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *TO();
    BlockContext *block();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BasicForStatementContext* basicForStatement();

  class  CstyleForStatementContext : public antlr4::ParserRuleContext {
  public:
    CstyleForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SEMI();
    antlr4::tree::TerminalNode* SEMI(size_t i);
    antlr4::tree::TerminalNode *RPAREN();
    BlockContext *block();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CstyleForStatementContext* cstyleForStatement();

  class  IdentifierListContext : public antlr4::ParserRuleContext {
  public:
    IdentifierListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COMMA();
    IdentifierListContext *identifierList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierListContext* identifierList();

  class  VariableDeclarationListContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VariableDeclarationContext *> variableDeclaration();
    VariableDeclarationContext* variableDeclaration(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationListContext* variableDeclarationList();

  class  ConstantDeclarationContext : public antlr4::ParserRuleContext {
  public:
    ConstantDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    VariableDeclarationInitializerContext *variableDeclarationInitializer();
    TypeAnnotationContext *typeAnnotation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstantDeclarationContext* constantDeclaration();

  class  VariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    TypeAnnotationContext *typeAnnotation();
    VariableDeclarationInitializerContext *variableDeclarationInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationContext* variableDeclaration();

  class  ProgramParametersContext : public antlr4::ParserRuleContext {
  public:
    ProgramParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ProgramParameterListContext *programParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramParametersContext* programParameters();

  class  ProgramParameterListContext : public antlr4::ParserRuleContext {
  public:
    ProgramParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ProgramParameterContext *> programParameter();
    ProgramParameterContext* programParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramParameterListContext* programParameterList();

  class  ProgramParameterContext : public antlr4::ParserRuleContext {
  public:
    ProgramParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNUSED();
    antlr4::tree::TerminalNode *IDENTIFIER();
    TypeAnnotationContext *typeAnnotation();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramParameterContext* programParameter();

  class  FunctionParametersContext : public antlr4::ParserRuleContext {
  public:
    FunctionParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    FunctionParameterListContext *functionParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionParametersContext* functionParameters();

  class  FunctionParameterListContext : public antlr4::ParserRuleContext {
  public:
    FunctionParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FunctionParameterContext *> functionParameter();
    FunctionParameterContext* functionParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionParameterListContext* functionParameterList();

  class  FunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    FunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *BYREF();
    antlr4::tree::TerminalNode *UNUSED();
    TypeAnnotationContext *typeAnnotation();
    antlr4::tree::TerminalNode *ELLIPSIS();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionParameterContext* functionParameter();

  class  ScopedFunctionCallContext : public antlr4::ParserRuleContext {
  public:
    ScopedFunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLONCOLON();
    FunctionCallContext *functionCall();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ScopedFunctionCallContext* scopedFunctionCall();

  class  FunctionReferenceContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *scope = nullptr;
    antlr4::Token *function = nullptr;
    FunctionReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    antlr4::tree::TerminalNode *COLONCOLON();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionReferenceContext* functionReference();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *prefix = nullptr;
    antlr4::Token *bop = nullptr;
    antlr4::Token *postfix = nullptr;
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryContext *primary();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *ADD();
    antlr4::tree::TerminalNode *SUB();
    antlr4::tree::TerminalNode *INC();
    antlr4::tree::TerminalNode *DEC();
    antlr4::tree::TerminalNode *TILDE();
    antlr4::tree::TerminalNode *BANG_A();
    antlr4::tree::TerminalNode *BANG_B();
    antlr4::tree::TerminalNode *MUL();
    antlr4::tree::TerminalNode *DIV();
    antlr4::tree::TerminalNode *MOD();
    antlr4::tree::TerminalNode *LSHIFT();
    std::vector<antlr4::tree::TerminalNode *> GT();
    antlr4::tree::TerminalNode* GT(size_t i);
    antlr4::tree::TerminalNode *BITAND();
    antlr4::tree::TerminalNode *BITOR();
    antlr4::tree::TerminalNode *CARET();
    antlr4::tree::TerminalNode *ELVIS();
    antlr4::tree::TerminalNode *TOK_IN();
    antlr4::tree::TerminalNode *LE();
    antlr4::tree::TerminalNode *GE();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *IS();
    antlr4::tree::TerminalNode *EQUAL_DEPRECATED();
    antlr4::tree::TerminalNode *EQUAL();
    antlr4::tree::TerminalNode *NOTEQUAL_B();
    antlr4::tree::TerminalNode *NOTEQUAL_A();
    antlr4::tree::TerminalNode *AND_A();
    antlr4::tree::TerminalNode *AND_B();
    antlr4::tree::TerminalNode *OR_A();
    antlr4::tree::TerminalNode *OR_B();
    antlr4::tree::TerminalNode *QUESTION();
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *ADDMEMBER();
    antlr4::tree::TerminalNode *DELMEMBER();
    antlr4::tree::TerminalNode *CHKMEMBER();
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *ADD_ASSIGN();
    antlr4::tree::TerminalNode *SUB_ASSIGN();
    antlr4::tree::TerminalNode *MUL_ASSIGN();
    antlr4::tree::TerminalNode *DIV_ASSIGN();
    antlr4::tree::TerminalNode *MOD_ASSIGN();
    ExpressionSuffixContext *expressionSuffix();
    antlr4::tree::TerminalNode *AS();
    TypeArgumentContext *typeArgument();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  PrimaryContext : public antlr4::ParserRuleContext {
  public:
    PrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LiteralContext *literal();
    ParExpressionContext *parExpression();
    FunctionCallContext *functionCall();
    ScopedFunctionCallContext *scopedFunctionCall();
    ScopedIdentifierContext *scopedIdentifier();
    antlr4::tree::TerminalNode *IDENTIFIER();
    FunctionReferenceContext *functionReference();
    FunctionExpressionContext *functionExpression();
    ExplicitArrayInitializerContext *explicitArrayInitializer();
    ExplicitStructInitializerContext *explicitStructInitializer();
    ExplicitDictInitializerContext *explicitDictInitializer();
    ExplicitErrorInitializerContext *explicitErrorInitializer();
    BareArrayInitializerContext *bareArrayInitializer();
    InterpolatedStringContext *interpolatedString();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryContext* primary();

  class  ScopedIdentifierContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *scope = nullptr;
    antlr4::Token *identifier = nullptr;
    ScopedIdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLONCOLON();
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ScopedIdentifierContext* scopedIdentifier();

  class  FunctionExpressionContext : public antlr4::ParserRuleContext {
  public:
    FunctionExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    antlr4::tree::TerminalNode *LBRACE();
    BlockContext *block();
    antlr4::tree::TerminalNode *RBRACE();
    FunctionParametersContext *functionParameters();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionExpressionContext* functionExpression();

  class  ExplicitArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    ExplicitArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ARRAY();
    ArrayInitializerContext *arrayInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExplicitArrayInitializerContext* explicitArrayInitializer();

  class  ExplicitStructInitializerContext : public antlr4::ParserRuleContext {
  public:
    ExplicitStructInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRUCT();
    StructInitializerContext *structInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExplicitStructInitializerContext* explicitStructInitializer();

  class  ExplicitDictInitializerContext : public antlr4::ParserRuleContext {
  public:
    ExplicitDictInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DICTIONARY();
    DictInitializerContext *dictInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExplicitDictInitializerContext* explicitDictInitializer();

  class  ExplicitErrorInitializerContext : public antlr4::ParserRuleContext {
  public:
    ExplicitErrorInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_ERROR();
    StructInitializerContext *structInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExplicitErrorInitializerContext* explicitErrorInitializer();

  class  BareArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    BareArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    ExpressionListContext *expressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BareArrayInitializerContext* bareArrayInitializer();

  class  ParExpressionContext : public antlr4::ParserRuleContext {
  public:
    ParExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParExpressionContext* parExpression();

  class  ExpressionListContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionListEntryContext *> expressionListEntry();
    ExpressionListEntryContext* expressionListEntry(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionListContext* expressionList();

  class  ExpressionListEntryContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListEntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *ELLIPSIS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionListEntryContext* expressionListEntry();

  class  ExpressionSuffixContext : public antlr4::ParserRuleContext {
  public:
    ExpressionSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IndexingSuffixContext *indexingSuffix();
    MethodCallSuffixContext *methodCallSuffix();
    NavigationSuffixContext *navigationSuffix();
    FunctionCallSuffixContext *functionCallSuffix();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionSuffixContext* expressionSuffix();

  class  IndexingSuffixContext : public antlr4::ParserRuleContext {
  public:
    IndexingSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACK();
    IndexListContext *indexList();
    antlr4::tree::TerminalNode *RBRACK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexingSuffixContext* indexingSuffix();

  class  IndexListContext : public antlr4::ParserRuleContext {
  public:
    IndexListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexListContext* indexList();

  class  NavigationSuffixContext : public antlr4::ParserRuleContext {
  public:
    NavigationSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *FUNCTION();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NavigationSuffixContext* navigationSuffix();

  class  MethodCallSuffixContext : public antlr4::ParserRuleContext {
  public:
    MethodCallSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ExpressionListContext *expressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MethodCallSuffixContext* methodCallSuffix();

  class  FunctionCallSuffixContext : public antlr4::ParserRuleContext {
  public:
    FunctionCallSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ExpressionListContext *expressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionCallSuffixContext* functionCallSuffix();

  class  FunctionCallContext : public antlr4::ParserRuleContext {
  public:
    FunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ExpressionListContext *expressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionCallContext* functionCall();

  class  StructInitializerExpressionContext : public antlr4::ParserRuleContext {
  public:
    StructInitializerExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructInitializerExpressionContext* structInitializerExpression();

  class  StructInitializerExpressionListContext : public antlr4::ParserRuleContext {
  public:
    StructInitializerExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StructInitializerExpressionContext *> structInitializerExpression();
    StructInitializerExpressionContext* structInitializerExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructInitializerExpressionListContext* structInitializerExpressionList();

  class  StructInitializerContext : public antlr4::ParserRuleContext {
  public:
    StructInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    StructInitializerExpressionListContext *structInitializerExpressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructInitializerContext* structInitializer();

  class  DictInitializerExpressionContext : public antlr4::ParserRuleContext {
  public:
    DictInitializerExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *ARROW();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerExpressionContext* dictInitializerExpression();

  class  DictInitializerExpressionListContext : public antlr4::ParserRuleContext {
  public:
    DictInitializerExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<DictInitializerExpressionContext *> dictInitializerExpression();
    DictInitializerExpressionContext* dictInitializerExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerExpressionListContext* dictInitializerExpressionList();

  class  DictInitializerContext : public antlr4::ParserRuleContext {
  public:
    DictInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    DictInitializerExpressionListContext *dictInitializerExpressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerContext* dictInitializer();

  class  ArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    ArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    ExpressionListContext *expressionList();
    antlr4::tree::TerminalNode *COMMA();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArrayInitializerContext* arrayInitializer();

  class  LiteralContext : public antlr4::ParserRuleContext {
  public:
    LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IntegerLiteralContext *integerLiteral();
    FloatLiteralContext *floatLiteral();
    BoolLiteralContext *boolLiteral();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *UNINIT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LiteralContext* literal();

  class  InterpolatedStringContext : public antlr4::ParserRuleContext {
  public:
    InterpolatedStringContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTERPOLATED_STRING_START();
    antlr4::tree::TerminalNode *DOUBLE_QUOTE_INSIDE();
    std::vector<InterpolatedStringPartContext *> interpolatedStringPart();
    InterpolatedStringPartContext* interpolatedStringPart(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InterpolatedStringContext* interpolatedString();

  class  InterpolatedStringPartContext : public antlr4::ParserRuleContext {
  public:
    InterpolatedStringPartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE_INSIDE();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *FORMAT_STRING();
    antlr4::tree::TerminalNode *DOUBLE_LBRACE_INSIDE();
    antlr4::tree::TerminalNode *REGULAR_CHAR_INSIDE();
    antlr4::tree::TerminalNode *DOUBLE_RBRACE();
    antlr4::tree::TerminalNode *STRING_LITERAL_INSIDE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InterpolatedStringPartContext* interpolatedStringPart();

  class  IntegerLiteralContext : public antlr4::ParserRuleContext {
  public:
    IntegerLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DECIMAL_LITERAL();
    antlr4::tree::TerminalNode *HEX_LITERAL();
    antlr4::tree::TerminalNode *OCT_LITERAL();
    antlr4::tree::TerminalNode *BINARY_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IntegerLiteralContext* integerLiteral();

  class  FloatLiteralContext : public antlr4::ParserRuleContext {
  public:
    FloatLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FLOAT_LITERAL();
    antlr4::tree::TerminalNode *HEX_FLOAT_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FloatLiteralContext* floatLiteral();

  class  BoolLiteralContext : public antlr4::ParserRuleContext {
  public:
    BoolLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BOOL_TRUE();
    antlr4::tree::TerminalNode *BOOL_FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BoolLiteralContext* boolLiteral();

  class  TypeParametersContext : public antlr4::ParserRuleContext {
  public:
    TypeParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LT();
    TypeParameterListContext *typeParameterList();
    antlr4::tree::TerminalNode *GT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeParametersContext* typeParameters();

  class  TypeParameterListContext : public antlr4::ParserRuleContext {
  public:
    TypeParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeParameterContext *> typeParameter();
    TypeParameterContext* typeParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeParameterListContext* typeParameterList();

  class  TypeParameterContext : public antlr4::ParserRuleContext {
  public:
    TypeParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    ConstraintContext *constraint();
    antlr4::tree::TerminalNode *ASSIGN();
    TypeArgumentContext *typeArgument();
    TypeParametersContext *typeParameters();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeParameterContext* typeParameter();

  class  ConstraintContext : public antlr4::ParserRuleContext {
  public:
    ConstraintContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    TypeContext *type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstraintContext* constraint();

  class  TypeArgumentsContext : public antlr4::ParserRuleContext {
  public:
    TypeArgumentsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *GT();
    TypeArgumentListContext *typeArgumentList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeArgumentsContext* typeArguments();

  class  TypeArgumentListContext : public antlr4::ParserRuleContext {
  public:
    TypeArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeArgumentContext *> typeArgument();
    TypeArgumentContext* typeArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeArgumentListContext* typeArgumentList();

  class  TypeArgumentContext : public antlr4::ParserRuleContext {
  public:
    TypeArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeArgumentContext* typeArgument();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BinaryOrPrimaryTypeContext *binaryOrPrimaryType();
    FunctionTypeContext *functionType();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();

  class  BinaryOrPrimaryTypeContext : public antlr4::ParserRuleContext {
  public:
    BinaryOrPrimaryTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryTypeContext *primaryType();
    std::vector<BinaryOrPrimaryTypeContext *> binaryOrPrimaryType();
    BinaryOrPrimaryTypeContext* binaryOrPrimaryType(size_t i);
    antlr4::tree::TerminalNode *BITOR();
    antlr4::tree::TerminalNode *BITAND();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BinaryOrPrimaryTypeContext* binaryOrPrimaryType();
  BinaryOrPrimaryTypeContext* binaryOrPrimaryType(int precedence);
  class  PrimaryTypeContext : public antlr4::ParserRuleContext {
  public:
    PrimaryTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    TypeContext *type();
    antlr4::tree::TerminalNode *RPAREN();
    PredefinedTypeContext *predefinedType();
    ObjectTypeContext *objectType();
    TypeReferenceContext *typeReference();
    antlr4::tree::TerminalNode *LBRACK();
    TupleElementTypesContext *tupleElementTypes();
    antlr4::tree::TerminalNode *RBRACK();
    PrimaryTypeContext *primaryType();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryTypeContext* primaryType();
  PrimaryTypeContext* primaryType(int precedence);
  class  PredefinedTypeContext : public antlr4::ParserRuleContext {
  public:
    PredefinedTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTEGER();
    antlr4::tree::TerminalNode *DOUBLE();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *UNINIT();
    antlr4::tree::TerminalNode *ARRAY();
    antlr4::tree::TerminalNode *TOK_LONG();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PredefinedTypeContext* predefinedType();

  class  TypeReferenceContext : public antlr4::ParserRuleContext {
  public:
    TypeReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdentifierNameContext *identifierName();
    TypeGenericContext *typeGeneric();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeReferenceContext* typeReference();

  class  TypeGenericContext : public antlr4::ParserRuleContext {
  public:
    TypeGenericContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LT();
    TypeArgumentListContext *typeArgumentList();
    antlr4::tree::TerminalNode *GT();
    TypeGenericContext *typeGeneric();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeGenericContext* typeGeneric();

  class  ObjectTypeContext : public antlr4::ParserRuleContext {
  public:
    ObjectTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRUCT();
    antlr4::tree::TerminalNode *DICTIONARY();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    TypeBodyContext *typeBody();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ObjectTypeContext* objectType();

  class  TypeBodyContext : public antlr4::ParserRuleContext {
  public:
    TypeBodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeMemberListContext *typeMemberList();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeBodyContext* typeBody();

  class  TypeMemberListContext : public antlr4::ParserRuleContext {
  public:
    TypeMemberListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeMemberContext *> typeMember();
    TypeMemberContext* typeMember(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SEMI();
    antlr4::tree::TerminalNode* SEMI(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeMemberListContext* typeMemberList();

  class  TypeMemberContext : public antlr4::ParserRuleContext {
  public:
    TypeMemberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertySignatureContext *propertySignature();
    CallSignatureContext *callSignature();
    IndexSignatureContext *indexSignature();
    MethodSignatureContext *methodSignature();
    antlr4::tree::TerminalNode *ARROW();
    TypeContext *type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeMemberContext* typeMember();

  class  TupleElementTypesContext : public antlr4::ParserRuleContext {
  public:
    TupleElementTypesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeContext *> type();
    TypeContext* type(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TupleElementTypesContext* tupleElementTypes();

  class  FunctionTypeContext : public antlr4::ParserRuleContext {
  public:
    FunctionTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *ARROW();
    TypeContext *type();
    TypeParametersContext *typeParameters();
    ParameterListContext *parameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionTypeContext* functionType();

  class  IdentifierNameContext : public antlr4::ParserRuleContext {
  public:
    IdentifierNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    ReservedWordContext *reservedWord();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierNameContext* identifierName();

  class  ReservedWordContext : public antlr4::ParserRuleContext {
  public:
    ReservedWordContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    antlr4::tree::TerminalNode *THEN();
    antlr4::tree::TerminalNode *ELSEIF();
    antlr4::tree::TerminalNode *ENDIF();
    antlr4::tree::TerminalNode *ELSE();
    antlr4::tree::TerminalNode *GOTO();
    antlr4::tree::TerminalNode *RETURN();
    antlr4::tree::TerminalNode *TOK_CONST();
    antlr4::tree::TerminalNode *VAR();
    antlr4::tree::TerminalNode *DO();
    antlr4::tree::TerminalNode *DOWHILE();
    antlr4::tree::TerminalNode *WHILE();
    antlr4::tree::TerminalNode *ENDWHILE();
    antlr4::tree::TerminalNode *EXIT();
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *ENDFUNCTION();
    antlr4::tree::TerminalNode *EXPORTED();
    antlr4::tree::TerminalNode *USE();
    antlr4::tree::TerminalNode *INCLUDE();
    antlr4::tree::TerminalNode *BREAK();
    antlr4::tree::TerminalNode *CONTINUE();
    antlr4::tree::TerminalNode *FOR();
    antlr4::tree::TerminalNode *ENDFOR();
    antlr4::tree::TerminalNode *TO();
    antlr4::tree::TerminalNode *FOREACH();
    antlr4::tree::TerminalNode *ENDFOREACH();
    antlr4::tree::TerminalNode *REPEAT();
    antlr4::tree::TerminalNode *UNTIL();
    antlr4::tree::TerminalNode *PROGRAM();
    antlr4::tree::TerminalNode *ENDPROGRAM();
    antlr4::tree::TerminalNode *CASE();
    antlr4::tree::TerminalNode *DEFAULT();
    antlr4::tree::TerminalNode *ENDCASE();
    antlr4::tree::TerminalNode *ENUM();
    antlr4::tree::TerminalNode *ENDENUM();
    antlr4::tree::TerminalNode *CLASS();
    antlr4::tree::TerminalNode *ENDCLASS();
    antlr4::tree::TerminalNode *DOWNTO();
    antlr4::tree::TerminalNode *STEP();
    antlr4::tree::TerminalNode *REFERENCE();
    antlr4::tree::TerminalNode *TOK_OUT();
    antlr4::tree::TerminalNode *INOUT();
    antlr4::tree::TerminalNode *BYVAL();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *TOK_LONG();
    antlr4::tree::TerminalNode *INTEGER();
    antlr4::tree::TerminalNode *UNSIGNED();
    antlr4::tree::TerminalNode *SIGNED();
    antlr4::tree::TerminalNode *REAL();
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *DOUBLE();
    antlr4::tree::TerminalNode *AS();
    antlr4::tree::TerminalNode *AND_B();
    antlr4::tree::TerminalNode *OR_B();
    antlr4::tree::TerminalNode *BANG_B();
    antlr4::tree::TerminalNode *BYREF();
    antlr4::tree::TerminalNode *UNUSED();
    antlr4::tree::TerminalNode *TOK_ERROR();
    antlr4::tree::TerminalNode *HASH();
    antlr4::tree::TerminalNode *DICTIONARY();
    antlr4::tree::TerminalNode *STRUCT();
    antlr4::tree::TerminalNode *ARRAY();
    antlr4::tree::TerminalNode *STACK();
    antlr4::tree::TerminalNode *TOK_IN();
    antlr4::tree::TerminalNode *UNINIT();
    antlr4::tree::TerminalNode *BOOL_TRUE();
    antlr4::tree::TerminalNode *BOOL_FALSE();
    antlr4::tree::TerminalNode *IS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReservedWordContext* reservedWord();

  class  PropertySignatureContext : public antlr4::ParserRuleContext {
  public:
    PropertySignatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyNameContext *propertyName();
    antlr4::tree::TerminalNode *QUESTION();
    TypeAnnotationContext *typeAnnotation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertySignatureContext* propertySignature();

  class  PropertyNameContext : public antlr4::ParserRuleContext {
  public:
    PropertyNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    ReservedWordContext *reservedWord();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertyNameContext* propertyName();

  class  TypeAnnotationContext : public antlr4::ParserRuleContext {
  public:
    TypeAnnotationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    TypeContext *type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeAnnotationContext* typeAnnotation();

  class  CallSignatureContext : public antlr4::ParserRuleContext {
  public:
    CallSignatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    TypeParametersContext *typeParameters();
    ParameterListContext *parameterList();
    TypeAnnotationContext *typeAnnotation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallSignatureContext* callSignature();

  class  ParameterListContext : public antlr4::ParserRuleContext {
  public:
    ParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    RestParameterContext *restParameter();
    std::vector<ParameterContext *> parameter();
    ParameterContext* parameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterListContext* parameterList();

  class  ParameterContext : public antlr4::ParserRuleContext {
  public:
    ParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ELVIS();
    TypeContext *type();
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *QUESTION();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterContext* parameter();

  class  RestParameterContext : public antlr4::ParserRuleContext {
  public:
    RestParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ELLIPSIS();
    TypeAnnotationContext *typeAnnotation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RestParameterContext* restParameter();

  class  IndexSignatureContext : public antlr4::ParserRuleContext {
  public:
    IndexSignatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACK();
    ParameterListContext *parameterList();
    antlr4::tree::TerminalNode *RBRACK();
    TypeAnnotationContext *typeAnnotation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexSignatureContext* indexSignature();

  class  MethodSignatureContext : public antlr4::ParserRuleContext {
  public:
    MethodSignatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyNameContext *propertyName();
    CallSignatureContext *callSignature();
    antlr4::tree::TerminalNode *QUESTION();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MethodSignatureContext* methodSignature();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);
  bool binaryOrPrimaryTypeSempred(BinaryOrPrimaryTypeContext *_localctx, size_t predicateIndex);
  bool primaryTypeSempred(PrimaryTypeContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

}  // namespace EscriptGrammar
