


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
    DOWNTO = 36, STEP = 37, REFERENCE = 38, TOK_OUT = 39, INOUT = 40, BYVAL = 41,
    STRING = 42, TOK_LONG = 43, INTEGER = 44, UNSIGNED = 45, SIGNED = 46,
    REAL = 47, FLOAT = 48, DOUBLE = 49, AS = 50, IS = 51, ELLIPSIS = 52,
    AND_A = 53, AND_B = 54, OR_A = 55, OR_B = 56, BANG_A = 57, BANG_B = 58,
    BYREF = 59, UNUSED = 60, TOK_ERROR = 61, HASH = 62, DICTIONARY = 63,
    STRUCT = 64, ARRAY = 65, STACK = 66, TOK_IN = 67, UNINIT = 68, BOOL_TRUE = 69,
    BOOL_FALSE = 70, DECIMAL_LITERAL = 71, HEX_LITERAL = 72, OCT_LITERAL = 73,
    BINARY_LITERAL = 74, FLOAT_LITERAL = 75, HEX_FLOAT_LITERAL = 76, STRING_LITERAL = 77,
    INTERPOLATED_STRING_START = 78, LPAREN = 79, RPAREN = 80, LBRACK = 81,
    RBRACK = 82, LBRACE = 83, RBRACE = 84, DOT = 85, ARROW = 86, MUL = 87,
    DIV = 88, MOD = 89, ADD = 90, SUB = 91, ADD_ASSIGN = 92, SUB_ASSIGN = 93,
    MUL_ASSIGN = 94, DIV_ASSIGN = 95, MOD_ASSIGN = 96, LE = 97, LT = 98,
    GE = 99, GT = 100, RSHIFT = 101, LSHIFT = 102, BITAND = 103, CARET = 104,
    BITOR = 105, NOTEQUAL_A = 106, NOTEQUAL_B = 107, EQUAL_DEPRECATED = 108,
    EQUAL = 109, ASSIGN = 110, ADDMEMBER = 111, DELMEMBER = 112, CHKMEMBER = 113,
    SEMI = 114, COMMA = 115, TILDE = 116, AT = 117, COLONCOLON = 118, COLON = 119,
    INC = 120, DEC = 121, ELVIS = 122, QUESTION = 123, WS = 124, COMMENT = 125,
    LINE_COMMENT = 126, IDENTIFIER = 127, DOUBLE_LBRACE_INSIDE = 128, LBRACE_INSIDE = 129,
    REGULAR_CHAR_INSIDE = 130, DOUBLE_QUOTE_INSIDE = 131, DOUBLE_RBRACE = 132,
    STRING_LITERAL_INSIDE = 133, CLOSE_RBRACE_INSIDE = 134, FORMAT_STRING = 135
  };

  enum {
    RuleCompilationUnit = 0, RuleModuleUnit = 1, RuleEvaluateUnit = 2, RuleModuleDeclarationStatement = 3,
    RuleModuleFunctionDeclaration = 4, RuleModuleFunctionParameterList = 5,
    RuleModuleFunctionParameter = 6, RuleTopLevelDeclaration = 7, RuleFunctionDeclaration = 8,
    RuleStringIdentifier = 9, RuleUseDeclaration = 10, RuleIncludeDeclaration = 11,
    RuleProgramDeclaration = 12, RuleStatement = 13, RuleStatementLabel = 14,
    RuleIfStatement = 15, RuleGotoStatement = 16, RuleReturnStatement = 17,
    RuleConstStatement = 18, RuleVarStatement = 19, RuleDoStatement = 20,
    RuleWhileStatement = 21, RuleExitStatement = 22, RuleBreakStatement = 23,
    RuleContinueStatement = 24, RuleForStatement = 25, RuleForeachIterableExpression = 26,
    RuleForeachStatement = 27, RuleRepeatStatement = 28, RuleCaseStatement = 29,
    RuleEnumStatement = 30, RuleBlock = 31, RuleVariableDeclarationInitializer = 32,
    RuleEnumList = 33, RuleEnumListEntry = 34, RuleSwitchBlockStatementGroup = 35,
    RuleSwitchLabel = 36, RuleForGroup = 37, RuleBasicForStatement = 38,
    RuleCstyleForStatement = 39, RuleIdentifierList = 40, RuleVariableDeclarationList = 41,
    RuleConstantDeclaration = 42, RuleVariableDeclaration = 43, RuleProgramParameters = 44,
    RuleProgramParameterList = 45, RuleProgramParameter = 46, RuleFunctionParameters = 47,
    RuleFunctionParameterList = 48, RuleFunctionParameter = 49, RuleScopedFunctionCall = 50,
    RuleFunctionReference = 51, RuleExpression = 52, RulePrimary = 53, RuleFunctionExpression = 54,
    RuleExplicitArrayInitializer = 55, RuleExplicitStructInitializer = 56,
    RuleExplicitDictInitializer = 57, RuleExplicitErrorInitializer = 58,
    RuleBareArrayInitializer = 59, RuleParExpression = 60, RuleExpressionList = 61,
    RuleExpressionListEntry = 62, RuleExpressionSuffix = 63, RuleIndexingSuffix = 64,
    RuleIndexList = 65, RuleNavigationSuffix = 66, RuleMethodCallSuffix = 67,
    RuleFunctionCallSuffix = 68, RuleFunctionCall = 69, RuleStructInitializerExpression = 70,
    RuleStructInitializerExpressionList = 71, RuleStructInitializer = 72,
    RuleDictInitializerExpression = 73, RuleDictInitializerExpressionList = 74,
    RuleDictInitializer = 75, RuleArrayInitializer = 76, RuleLiteral = 77,
    RuleInterpolatedString = 78, RuleInterpolatedStringPart = 79, RuleIntegerLiteral = 80,
    RuleFloatLiteral = 81, RuleBoolLiteral = 82
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
    StatementContext *statement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  TopLevelDeclarationContext* topLevelDeclaration();

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
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COLONCOLON();
    FunctionCallContext *functionCall();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;

  };

  ScopedFunctionCallContext* scopedFunctionCall();

  class  FunctionReferenceContext : public antlr4::ParserRuleContext {
  public:
    FunctionReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    antlr4::tree::TerminalNode *IDENTIFIER();

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
    antlr4::tree::TerminalNode *RSHIFT();
    antlr4::tree::TerminalNode *BITAND();
    antlr4::tree::TerminalNode *BITOR();
    antlr4::tree::TerminalNode *CARET();
    antlr4::tree::TerminalNode *ELVIS();
    antlr4::tree::TerminalNode *TOK_IN();
    antlr4::tree::TerminalNode *LE();
    antlr4::tree::TerminalNode *GE();
    antlr4::tree::TerminalNode *GT();
    antlr4::tree::TerminalNode *LT();
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


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

}  // namespace EscriptGrammar
