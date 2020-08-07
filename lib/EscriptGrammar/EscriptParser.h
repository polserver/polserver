


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"


namespace EscriptGrammar {


class  EscriptParser : public antlr4::Parser {
public:
  enum {
    IF = 1, THEN = 2, ELSEIF = 3, ENDIF = 4, ELSE = 5, GOTO = 6, RETURN = 7, 
    TOK_CONST = 8, VAR = 9, DO = 10, DOWHILE = 11, WHILE = 12, ENDWHILE = 13, 
    EXIT = 14, DECLARE = 15, FUNCTION = 16, ENDFUNCTION = 17, EXPORTED = 18, 
    USE = 19, INCLUDE = 20, BREAK = 21, CONTINUE = 22, FOR = 23, ENDFOR = 24, 
    TO = 25, FOREACH = 26, ENDFOREACH = 27, REPEAT = 28, UNTIL = 29, PROGRAM = 30, 
    ENDPROGRAM = 31, CASE = 32, DEFAULT = 33, ENDCASE = 34, ENUM = 35, ENDENUM = 36, 
    DOWNTO = 37, STEP = 38, REFERENCE = 39, TOK_OUT = 40, INOUT = 41, BYVAL = 42, 
    STRING = 43, TOK_LONG = 44, INTEGER = 45, UNSIGNED = 46, SIGNED = 47, 
    REAL = 48, FLOAT = 49, DOUBLE = 50, AS = 51, IS = 52, AND_A = 53, AND_B = 54, 
    OR_A = 55, OR_B = 56, BANG_A = 57, BANG_B = 58, BYREF = 59, UNUSED = 60, 
    TOK_ERROR = 61, HASH = 62, DICTIONARY = 63, STRUCT = 64, ARRAY = 65, 
    STACK = 66, TOK_IN = 67, DECIMAL_LITERAL = 68, HEX_LITERAL = 69, OCT_LITERAL = 70, 
    BINARY_LITERAL = 71, FLOAT_LITERAL = 72, HEX_FLOAT_LITERAL = 73, CHAR_LITERAL = 74, 
    STRING_LITERAL = 75, NULL_LITERAL = 76, LPAREN = 77, RPAREN = 78, LBRACK = 79, 
    RBRACK = 80, LBRACE = 81, RBRACE = 82, DOT = 83, ARROW = 84, MUL = 85, 
    DIV = 86, MOD = 87, ADD = 88, SUB = 89, ADD_ASSIGN = 90, SUB_ASSIGN = 91, 
    MUL_ASSIGN = 92, DIV_ASSIGN = 93, MOD_ASSIGN = 94, LE = 95, LT = 96, 
    GE = 97, GT = 98, RSHIFT = 99, LSHIFT = 100, BITAND = 101, CARET = 102, 
    BITOR = 103, NOTEQUAL_A = 104, NOTEQUAL_B = 105, EQUAL_DEPRECATED = 106, 
    EQUAL = 107, ASSIGN = 108, ADDMEMBER = 109, DELMEMBER = 110, CHKMEMBER = 111, 
    SEMI = 112, COMMA = 113, TILDE = 114, AT = 115, COLONCOLON = 116, COLON = 117, 
    INC = 118, DEC = 119, ELVIS = 120, WS = 121, COMMENT = 122, LINE_COMMENT = 123, 
    IDENTIFIER = 124
  };

  enum {
    RuleUnambiguousCompilationUnit = 0, RuleCompilationUnit = 1, RuleModuleUnit = 2, 
    RuleUnambiguousModuleUnit = 3, RuleModuleDeclarationStatement = 4, RuleUnambiguousModuleDeclarationStatement = 5, 
    RuleModuleFunctionDeclaration = 6, RuleUnambiguousModuleFunctionDeclaration = 7, 
    RuleModuleFunctionParameterList = 8, RuleUnambiguousModuleFunctionParameterList = 9, 
    RuleModuleFunctionParameter = 10, RuleUnambiguousModuleFunctionParameter = 11, 
    RuleTopLevelDeclaration = 12, RuleUnambiguousTopLevelDeclaration = 13, 
    RuleFunctionDeclaration = 14, RuleUnambiguousFunctionDeclaration = 15, 
    RuleStringIdentifier = 16, RuleUseDeclaration = 17, RuleIncludeDeclaration = 18, 
    RuleProgramDeclaration = 19, RuleUnambiguousProgramDeclaration = 20, 
    RuleUnambiguousStatement = 21, RuleStatement = 22, RuleStatementLabel = 23, 
    RuleIfStatement = 24, RuleUnambiguousIfStatement = 25, RuleGotoStatement = 26, 
    RuleReturnStatement = 27, RuleUnambiguousReturnStatement = 28, RuleConstStatement = 29, 
    RuleUnambiguousConstStatement = 30, RuleVarStatement = 31, RuleUnambiguousVarStatement = 32, 
    RuleDoStatement = 33, RuleUnambiguousDoStatement = 34, RuleWhileStatement = 35, 
    RuleUnambiguousWhileStatement = 36, RuleExitStatement = 37, RuleDeclareStatement = 38, 
    RuleBreakStatement = 39, RuleContinueStatement = 40, RuleForStatement = 41, 
    RuleUnambiguousForStatement = 42, RuleForeachStatement = 43, RuleUnambiguousForeachStatement = 44, 
    RuleRepeatStatement = 45, RuleUnambiguousRepeatStatement = 46, RuleCaseStatement = 47, 
    RuleUnambiguousCaseStatement = 48, RuleEnumStatement = 49, RuleUnambiguousEnumStatement = 50, 
    RuleBlock = 51, RuleUnambiguousBlock = 52, RuleVariableDeclarationInitializer = 53, 
    RuleUnambiguousVariableDeclarationInitializer = 54, RuleEnumList = 55, 
    RuleUnambiguousEnumList = 56, RuleEnumListEntry = 57, RuleUnambiguousEnumListEntry = 58, 
    RuleSwitchBlockStatementGroup = 59, RuleUnambiguousSwitchBlockStatementGroup = 60, 
    RuleSwitchLabel = 61, RuleForGroup = 62, RuleUnambiguousForGroup = 63, 
    RuleBasicForStatement = 64, RuleUnambiguousBasicForStatement = 65, RuleCstyleForStatement = 66, 
    RuleUnambiguousCstyleForStatement = 67, RuleIdentifierList = 68, RuleVariableDeclarationList = 69, 
    RuleUnambiguousVariableDeclarationList = 70, RuleVariableDeclaration = 71, 
    RuleUnambiguousVariableDeclaration = 72, RuleProgramParameters = 73, 
    RuleProgramParameterList = 74, RuleProgramParameter = 75, RuleUnambiguousProgramParameters = 76, 
    RuleUnambiguousProgramParameterList = 77, RuleUnambiguousProgramParameter = 78, 
    RuleFunctionParameters = 79, RuleFunctionParameterList = 80, RuleFunctionParameter = 81, 
    RuleUnambiguousFunctionParameters = 82, RuleUnambiguousFunctionParameterList = 83, 
    RuleUnambiguousFunctionParameter = 84, RuleScopedMethodCall = 85, RuleUnambiguousExpression = 86, 
    RuleAssignmentOperator = 87, RuleMembership = 88, RuleMembershipOperator = 89, 
    RuleDisjunction = 90, RuleDisjunctionOperator = 91, RuleConjunction = 92, 
    RuleConjunctionOperator = 93, RuleEquality = 94, RuleEqualityOperator = 95, 
    RuleBitwiseDisjunction = 96, RuleBitwiseXor = 97, RuleBitwiseConjunction = 98, 
    RuleBitshiftRight = 99, RuleBitshiftLeft = 100, RuleComparison = 101, 
    RuleComparisonOperator = 102, RuleInfixOperation = 103, RuleInfixOperator = 104, 
    RuleElvisExpression = 105, RuleAdditiveExpression = 106, RuleAdditiveOperator = 107, 
    RuleMultiplicativeExpression = 108, RuleMultiplicativeOperator = 109, 
    RulePrefixUnaryInversionExpression = 110, RulePrefixUnaryInversionOperator = 111, 
    RulePrefixUnaryArithmeticExpression = 112, RulePrefixUnaryArithmeticOperator = 113, 
    RulePostfixUnaryExpression = 114, RulePostfixUnaryOperator = 115, RuleIndexingSuffix = 116, 
    RuleNavigationSuffix = 117, RuleMembershipSuffix = 118, RuleMemberCallSuffix = 119, 
    RuleMemberAccessOperator = 120, RuleCallSuffix = 121, RuleAtomicExpression = 122, 
    RuleFunctionReference = 123, RuleUnambiguousExplicitArrayInitializer = 124, 
    RuleUnambiguousExplicitStructInitializer = 125, RuleUnambiguousExplicitDictInitializer = 126, 
    RuleUnambiguousExplicitErrorInitializer = 127, RuleUnambiguousBareArrayInitializer = 128, 
    RuleParenthesizedExpression = 129, RuleExpression = 130, RulePrimary = 131, 
    RuleParExpression = 132, RuleExpressionList = 133, RuleUnambiguousExpressionList = 134, 
    RuleMethodCallArgument = 135, RuleMethodCallArgumentList = 136, RuleMethodCall = 137, 
    RuleValueArgumentList = 138, RuleValueArguments = 139, RuleUnambiguousFunctionCallArgument = 140, 
    RuleUnambiguousFunctionCall = 141, RuleUnambiguousScopedFunctionCall = 142, 
    RuleUnambiguousFunctionCallArgumentList = 143, RuleMemberCall = 144, 
    RuleStructInitializerExpression = 145, RuleStructInitializerExpressionList = 146, 
    RuleStructInitializer = 147, RuleUnambiguousStructInitializerExpression = 148, 
    RuleUnambiguousStructInitializerExpressionList = 149, RuleUnambiguousStructInitializer = 150, 
    RuleDictInitializerExpression = 151, RuleUnambiguousDictInitializerExpression = 152, 
    RuleDictInitializerExpressionList = 153, RuleUnambiguousDictInitializerExpressionList = 154, 
    RuleDictInitializer = 155, RuleUnambiguousDictInitializer = 156, RuleArrayInitializer = 157, 
    RuleUnambiguousArrayInitializer = 158, RuleLiteral = 159, RuleIntegerLiteral = 160, 
    RuleFloatLiteral = 161
  };

  EscriptParser(antlr4::TokenStream *input);
  ~EscriptParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;




  class UnambiguousCompilationUnitContext;
  class CompilationUnitContext;
  class ModuleUnitContext;
  class UnambiguousModuleUnitContext;
  class ModuleDeclarationStatementContext;
  class UnambiguousModuleDeclarationStatementContext;
  class ModuleFunctionDeclarationContext;
  class UnambiguousModuleFunctionDeclarationContext;
  class ModuleFunctionParameterListContext;
  class UnambiguousModuleFunctionParameterListContext;
  class ModuleFunctionParameterContext;
  class UnambiguousModuleFunctionParameterContext;
  class TopLevelDeclarationContext;
  class UnambiguousTopLevelDeclarationContext;
  class FunctionDeclarationContext;
  class UnambiguousFunctionDeclarationContext;
  class StringIdentifierContext;
  class UseDeclarationContext;
  class IncludeDeclarationContext;
  class ProgramDeclarationContext;
  class UnambiguousProgramDeclarationContext;
  class UnambiguousStatementContext;
  class StatementContext;
  class StatementLabelContext;
  class IfStatementContext;
  class UnambiguousIfStatementContext;
  class GotoStatementContext;
  class ReturnStatementContext;
  class UnambiguousReturnStatementContext;
  class ConstStatementContext;
  class UnambiguousConstStatementContext;
  class VarStatementContext;
  class UnambiguousVarStatementContext;
  class DoStatementContext;
  class UnambiguousDoStatementContext;
  class WhileStatementContext;
  class UnambiguousWhileStatementContext;
  class ExitStatementContext;
  class DeclareStatementContext;
  class BreakStatementContext;
  class ContinueStatementContext;
  class ForStatementContext;
  class UnambiguousForStatementContext;
  class ForeachStatementContext;
  class UnambiguousForeachStatementContext;
  class RepeatStatementContext;
  class UnambiguousRepeatStatementContext;
  class CaseStatementContext;
  class UnambiguousCaseStatementContext;
  class EnumStatementContext;
  class UnambiguousEnumStatementContext;
  class BlockContext;
  class UnambiguousBlockContext;
  class VariableDeclarationInitializerContext;
  class UnambiguousVariableDeclarationInitializerContext;
  class EnumListContext;
  class UnambiguousEnumListContext;
  class EnumListEntryContext;
  class UnambiguousEnumListEntryContext;
  class SwitchBlockStatementGroupContext;
  class UnambiguousSwitchBlockStatementGroupContext;
  class SwitchLabelContext;
  class ForGroupContext;
  class UnambiguousForGroupContext;
  class BasicForStatementContext;
  class UnambiguousBasicForStatementContext;
  class CstyleForStatementContext;
  class UnambiguousCstyleForStatementContext;
  class IdentifierListContext;
  class VariableDeclarationListContext;
  class UnambiguousVariableDeclarationListContext;
  class VariableDeclarationContext;
  class UnambiguousVariableDeclarationContext;
  class ProgramParametersContext;
  class ProgramParameterListContext;
  class ProgramParameterContext;
  class UnambiguousProgramParametersContext;
  class UnambiguousProgramParameterListContext;
  class UnambiguousProgramParameterContext;
  class FunctionParametersContext;
  class FunctionParameterListContext;
  class FunctionParameterContext;
  class UnambiguousFunctionParametersContext;
  class UnambiguousFunctionParameterListContext;
  class UnambiguousFunctionParameterContext;
  class ScopedMethodCallContext;
  class UnambiguousExpressionContext;
  class AssignmentOperatorContext;
  class MembershipContext;
  class MembershipOperatorContext;
  class DisjunctionContext;
  class DisjunctionOperatorContext;
  class ConjunctionContext;
  class ConjunctionOperatorContext;
  class EqualityContext;
  class EqualityOperatorContext;
  class BitwiseDisjunctionContext;
  class BitwiseXorContext;
  class BitwiseConjunctionContext;
  class BitshiftRightContext;
  class BitshiftLeftContext;
  class ComparisonContext;
  class ComparisonOperatorContext;
  class InfixOperationContext;
  class InfixOperatorContext;
  class ElvisExpressionContext;
  class AdditiveExpressionContext;
  class AdditiveOperatorContext;
  class MultiplicativeExpressionContext;
  class MultiplicativeOperatorContext;
  class PrefixUnaryInversionExpressionContext;
  class PrefixUnaryInversionOperatorContext;
  class PrefixUnaryArithmeticExpressionContext;
  class PrefixUnaryArithmeticOperatorContext;
  class PostfixUnaryExpressionContext;
  class PostfixUnaryOperatorContext;
  class IndexingSuffixContext;
  class NavigationSuffixContext;
  class MembershipSuffixContext;
  class MemberCallSuffixContext;
  class MemberAccessOperatorContext;
  class CallSuffixContext;
  class AtomicExpressionContext;
  class FunctionReferenceContext;
  class UnambiguousExplicitArrayInitializerContext;
  class UnambiguousExplicitStructInitializerContext;
  class UnambiguousExplicitDictInitializerContext;
  class UnambiguousExplicitErrorInitializerContext;
  class UnambiguousBareArrayInitializerContext;
  class ParenthesizedExpressionContext;
  class ExpressionContext;
  class PrimaryContext;
  class ParExpressionContext;
  class ExpressionListContext;
  class UnambiguousExpressionListContext;
  class MethodCallArgumentContext;
  class MethodCallArgumentListContext;
  class MethodCallContext;
  class ValueArgumentListContext;
  class ValueArgumentsContext;
  class UnambiguousFunctionCallArgumentContext;
  class UnambiguousFunctionCallContext;
  class UnambiguousScopedFunctionCallContext;
  class UnambiguousFunctionCallArgumentListContext;
  class MemberCallContext;
  class StructInitializerExpressionContext;
  class StructInitializerExpressionListContext;
  class StructInitializerContext;
  class UnambiguousStructInitializerExpressionContext;
  class UnambiguousStructInitializerExpressionListContext;
  class UnambiguousStructInitializerContext;
  class DictInitializerExpressionContext;
  class UnambiguousDictInitializerExpressionContext;
  class DictInitializerExpressionListContext;
  class UnambiguousDictInitializerExpressionListContext;
  class DictInitializerContext;
  class UnambiguousDictInitializerContext;
  class ArrayInitializerContext;
  class UnambiguousArrayInitializerContext;
  class LiteralContext;
  class IntegerLiteralContext;
  class FloatLiteralContext; 

  class  UnambiguousCompilationUnitContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousCompilationUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<UnambiguousTopLevelDeclarationContext *> unambiguousTopLevelDeclaration();
    UnambiguousTopLevelDeclarationContext* unambiguousTopLevelDeclaration(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousCompilationUnitContext* unambiguousCompilationUnit();

  class  CompilationUnitContext : public antlr4::ParserRuleContext {
  public:
    CompilationUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<TopLevelDeclarationContext *> topLevelDeclaration();
    TopLevelDeclarationContext* topLevelDeclaration(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleUnitContext* moduleUnit();

  class  UnambiguousModuleUnitContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousModuleUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<UnambiguousModuleDeclarationStatementContext *> unambiguousModuleDeclarationStatement();
    UnambiguousModuleDeclarationStatementContext* unambiguousModuleDeclarationStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousModuleUnitContext* unambiguousModuleUnit();

  class  ModuleDeclarationStatementContext : public antlr4::ParserRuleContext {
  public:
    ModuleDeclarationStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ModuleFunctionDeclarationContext *moduleFunctionDeclaration();
    ConstStatementContext *constStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleDeclarationStatementContext* moduleDeclarationStatement();

  class  UnambiguousModuleDeclarationStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousModuleDeclarationStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UnambiguousModuleFunctionDeclarationContext *unambiguousModuleFunctionDeclaration();
    UnambiguousConstStatementContext *unambiguousConstStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousModuleDeclarationStatementContext* unambiguousModuleDeclarationStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionDeclarationContext* moduleFunctionDeclaration();

  class  UnambiguousModuleFunctionDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousModuleFunctionDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *SEMI();
    UnambiguousModuleFunctionParameterListContext *unambiguousModuleFunctionParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousModuleFunctionDeclarationContext* unambiguousModuleFunctionDeclaration();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionParameterListContext* moduleFunctionParameterList();

  class  UnambiguousModuleFunctionParameterListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousModuleFunctionParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousModuleFunctionParameterContext *> unambiguousModuleFunctionParameter();
    UnambiguousModuleFunctionParameterContext* unambiguousModuleFunctionParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousModuleFunctionParameterListContext* unambiguousModuleFunctionParameterList();

  class  ModuleFunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    ModuleFunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleFunctionParameterContext* moduleFunctionParameter();

  class  UnambiguousModuleFunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousModuleFunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousModuleFunctionParameterContext* unambiguousModuleFunctionParameter();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TopLevelDeclarationContext* topLevelDeclaration();

  class  UnambiguousTopLevelDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousTopLevelDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UseDeclarationContext *useDeclaration();
    IncludeDeclarationContext *includeDeclaration();
    UnambiguousProgramDeclarationContext *unambiguousProgramDeclaration();
    UnambiguousFunctionDeclarationContext *unambiguousFunctionDeclaration();
    UnambiguousStatementContext *unambiguousStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousTopLevelDeclarationContext* unambiguousTopLevelDeclaration();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionDeclarationContext* functionDeclaration();

  class  UnambiguousFunctionDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *IDENTIFIER();
    UnambiguousFunctionParametersContext *unambiguousFunctionParameters();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *ENDFUNCTION();
    antlr4::tree::TerminalNode *EXPORTED();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionDeclarationContext* unambiguousFunctionDeclaration();

  class  StringIdentifierContext : public antlr4::ParserRuleContext {
  public:
    StringIdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramDeclarationContext* programDeclaration();

  class  UnambiguousProgramDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousProgramDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PROGRAM();
    antlr4::tree::TerminalNode *IDENTIFIER();
    UnambiguousProgramParametersContext *unambiguousProgramParameters();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *ENDPROGRAM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousProgramDeclarationContext* unambiguousProgramDeclaration();

  class  UnambiguousStatementContext : public antlr4::ParserRuleContext {
  public:
    EscriptParser::UnambiguousExpressionContext *statementUnambiguousExpression = nullptr;;
    UnambiguousStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UnambiguousIfStatementContext *unambiguousIfStatement();
    UnambiguousReturnStatementContext *unambiguousReturnStatement();
    UnambiguousConstStatementContext *unambiguousConstStatement();
    UnambiguousVarStatementContext *unambiguousVarStatement();
    UnambiguousDoStatementContext *unambiguousDoStatement();
    UnambiguousWhileStatementContext *unambiguousWhileStatement();
    ExitStatementContext *exitStatement();
    BreakStatementContext *breakStatement();
    ContinueStatementContext *continueStatement();
    UnambiguousForStatementContext *unambiguousForStatement();
    UnambiguousForeachStatementContext *unambiguousForeachStatement();
    UnambiguousRepeatStatementContext *unambiguousRepeatStatement();
    UnambiguousCaseStatementContext *unambiguousCaseStatement();
    UnambiguousEnumStatementContext *unambiguousEnumStatement();
    antlr4::tree::TerminalNode *SEMI();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousStatementContext* unambiguousStatement();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    EscriptParser::ExpressionContext *statementExpression = nullptr;;
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
    DeclareStatementContext *declareStatement();
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IfStatementContext* ifStatement();

  class  UnambiguousIfStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousIfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    std::vector<ParenthesizedExpressionContext *> parenthesizedExpression();
    ParenthesizedExpressionContext* parenthesizedExpression(size_t i);
    std::vector<UnambiguousBlockContext *> unambiguousBlock();
    UnambiguousBlockContext* unambiguousBlock(size_t i);
    antlr4::tree::TerminalNode *ENDIF();
    antlr4::tree::TerminalNode *THEN();
    std::vector<antlr4::tree::TerminalNode *> ELSEIF();
    antlr4::tree::TerminalNode* ELSEIF(size_t i);
    antlr4::tree::TerminalNode *ELSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousIfStatementContext* unambiguousIfStatement();

  class  GotoStatementContext : public antlr4::ParserRuleContext {
  public:
    GotoStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GOTO();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnStatementContext* returnStatement();

  class  UnambiguousReturnStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousReturnStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURN();
    antlr4::tree::TerminalNode *SEMI();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousReturnStatementContext* unambiguousReturnStatement();

  class  ConstStatementContext : public antlr4::ParserRuleContext {
  public:
    ConstStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_CONST();
    VariableDeclarationContext *variableDeclaration();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstStatementContext* constStatement();

  class  UnambiguousConstStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousConstStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_CONST();
    UnambiguousVariableDeclarationContext *unambiguousVariableDeclaration();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousConstStatementContext* unambiguousConstStatement();

  class  VarStatementContext : public antlr4::ParserRuleContext {
  public:
    VarStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VAR();
    VariableDeclarationListContext *variableDeclarationList();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarStatementContext* varStatement();

  class  UnambiguousVarStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousVarStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VAR();
    UnambiguousVariableDeclarationListContext *unambiguousVariableDeclarationList();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousVarStatementContext* unambiguousVarStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DoStatementContext* doStatement();

  class  UnambiguousDoStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousDoStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DO();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *DOWHILE();
    ParenthesizedExpressionContext *parenthesizedExpression();
    antlr4::tree::TerminalNode *SEMI();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousDoStatementContext* unambiguousDoStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhileStatementContext* whileStatement();

  class  UnambiguousWhileStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousWhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WHILE();
    ParenthesizedExpressionContext *parenthesizedExpression();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *ENDWHILE();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousWhileStatementContext* unambiguousWhileStatement();

  class  ExitStatementContext : public antlr4::ParserRuleContext {
  public:
    ExitStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EXIT();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExitStatementContext* exitStatement();

  class  DeclareStatementContext : public antlr4::ParserRuleContext {
  public:
    DeclareStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DECLARE();
    antlr4::tree::TerminalNode *FUNCTION();
    IdentifierListContext *identifierList();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclareStatementContext* declareStatement();

  class  BreakStatementContext : public antlr4::ParserRuleContext {
  public:
    BreakStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BREAK();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForStatementContext* forStatement();

  class  UnambiguousForStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOR();
    UnambiguousForGroupContext *unambiguousForGroup();
    antlr4::tree::TerminalNode *ENDFOR();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousForStatementContext* unambiguousForStatement();

  class  ForeachStatementContext : public antlr4::ParserRuleContext {
  public:
    ForeachStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOREACH();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *TOK_IN();
    ExpressionContext *expression();
    BlockContext *block();
    antlr4::tree::TerminalNode *ENDFOREACH();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForeachStatementContext* foreachStatement();

  class  UnambiguousForeachStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousForeachStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FOREACH();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *TOK_IN();
    UnambiguousExpressionContext *unambiguousExpression();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *ENDFOREACH();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousForeachStatementContext* unambiguousForeachStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RepeatStatementContext* repeatStatement();

  class  UnambiguousRepeatStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousRepeatStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REPEAT();
    UnambiguousBlockContext *unambiguousBlock();
    antlr4::tree::TerminalNode *UNTIL();
    UnambiguousExpressionContext *unambiguousExpression();
    antlr4::tree::TerminalNode *SEMI();
    StatementLabelContext *statementLabel();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousRepeatStatementContext* unambiguousRepeatStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CaseStatementContext* caseStatement();

  class  UnambiguousCaseStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousCaseStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CASE();
    antlr4::tree::TerminalNode *LPAREN();
    UnambiguousExpressionContext *unambiguousExpression();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *ENDCASE();
    StatementLabelContext *statementLabel();
    std::vector<UnambiguousSwitchBlockStatementGroupContext *> unambiguousSwitchBlockStatementGroup();
    UnambiguousSwitchBlockStatementGroupContext* unambiguousSwitchBlockStatementGroup(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousCaseStatementContext* unambiguousCaseStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumStatementContext* enumStatement();

  class  UnambiguousEnumStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousEnumStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ENUM();
    antlr4::tree::TerminalNode *IDENTIFIER();
    UnambiguousEnumListContext *unambiguousEnumList();
    antlr4::tree::TerminalNode *ENDENUM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousEnumStatementContext* unambiguousEnumStatement();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  UnambiguousBlockContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousStatementContext *> unambiguousStatement();
    UnambiguousStatementContext* unambiguousStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousBlockContext* unambiguousBlock();

  class  VariableDeclarationInitializerContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *ARRAY();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationInitializerContext* variableDeclarationInitializer();

  class  UnambiguousVariableDeclarationInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousVariableDeclarationInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();
    antlr4::tree::TerminalNode *EQUAL_DEPRECATED();
    antlr4::tree::TerminalNode *ARRAY();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousVariableDeclarationInitializerContext* unambiguousVariableDeclarationInitializer();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumListContext* enumList();

  class  UnambiguousEnumListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousEnumListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousEnumListEntryContext *> unambiguousEnumListEntry();
    UnambiguousEnumListEntryContext* unambiguousEnumListEntry(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousEnumListContext* unambiguousEnumList();

  class  EnumListEntryContext : public antlr4::ParserRuleContext {
  public:
    EnumListEntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumListEntryContext* enumListEntry();

  class  UnambiguousEnumListEntryContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousEnumListEntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousEnumListEntryContext* unambiguousEnumListEntry();

  class  SwitchBlockStatementGroupContext : public antlr4::ParserRuleContext {
  public:
    SwitchBlockStatementGroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BlockContext *block();
    std::vector<SwitchLabelContext *> switchLabel();
    SwitchLabelContext* switchLabel(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SwitchBlockStatementGroupContext* switchBlockStatementGroup();

  class  UnambiguousSwitchBlockStatementGroupContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousSwitchBlockStatementGroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SwitchLabelContext *> switchLabel();
    SwitchLabelContext* switchLabel(size_t i);
    UnambiguousBlockContext *unambiguousBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousSwitchBlockStatementGroupContext* unambiguousSwitchBlockStatementGroup();

  class  SwitchLabelContext : public antlr4::ParserRuleContext {
  public:
    SwitchLabelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    IntegerLiteralContext *integerLiteral();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *DEFAULT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForGroupContext* forGroup();

  class  UnambiguousForGroupContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousForGroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UnambiguousCstyleForStatementContext *unambiguousCstyleForStatement();
    UnambiguousBasicForStatementContext *unambiguousBasicForStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousForGroupContext* unambiguousForGroup();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BasicForStatementContext* basicForStatement();

  class  UnambiguousBasicForStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousBasicForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    std::vector<UnambiguousExpressionContext *> unambiguousExpression();
    UnambiguousExpressionContext* unambiguousExpression(size_t i);
    antlr4::tree::TerminalNode *TO();
    UnambiguousBlockContext *unambiguousBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousBasicForStatementContext* unambiguousBasicForStatement();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CstyleForStatementContext* cstyleForStatement();

  class  UnambiguousCstyleForStatementContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousCstyleForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    std::vector<UnambiguousExpressionContext *> unambiguousExpression();
    UnambiguousExpressionContext* unambiguousExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SEMI();
    antlr4::tree::TerminalNode* SEMI(size_t i);
    antlr4::tree::TerminalNode *RPAREN();
    UnambiguousBlockContext *unambiguousBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousCstyleForStatementContext* unambiguousCstyleForStatement();

  class  IdentifierListContext : public antlr4::ParserRuleContext {
  public:
    IdentifierListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COMMA();
    IdentifierListContext *identifierList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationListContext* variableDeclarationList();

  class  UnambiguousVariableDeclarationListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousVariableDeclarationListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousVariableDeclarationContext *> unambiguousVariableDeclaration();
    UnambiguousVariableDeclarationContext* unambiguousVariableDeclaration(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousVariableDeclarationListContext* unambiguousVariableDeclarationList();

  class  VariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    VariableDeclarationInitializerContext *variableDeclarationInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationContext* variableDeclaration();

  class  UnambiguousVariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousVariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    UnambiguousVariableDeclarationInitializerContext *unambiguousVariableDeclarationInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousVariableDeclarationContext* unambiguousVariableDeclaration();

  class  ProgramParametersContext : public antlr4::ParserRuleContext {
  public:
    ProgramParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ProgramParameterListContext *programParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramParameterContext* programParameter();

  class  UnambiguousProgramParametersContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousProgramParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    UnambiguousProgramParameterListContext *unambiguousProgramParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousProgramParametersContext* unambiguousProgramParameters();

  class  UnambiguousProgramParameterListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousProgramParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousProgramParameterContext *> unambiguousProgramParameter();
    UnambiguousProgramParameterContext* unambiguousProgramParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousProgramParameterListContext* unambiguousProgramParameterList();

  class  UnambiguousProgramParameterContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousProgramParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNUSED();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousProgramParameterContext* unambiguousProgramParameter();

  class  FunctionParametersContext : public antlr4::ParserRuleContext {
  public:
    FunctionParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    FunctionParameterListContext *functionParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionParameterListContext* functionParameterList();

  class  FunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    FunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *BYREF();
    antlr4::tree::TerminalNode *UNUSED();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionParameterContext* functionParameter();

  class  UnambiguousFunctionParametersContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionParametersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    UnambiguousFunctionParameterListContext *unambiguousFunctionParameterList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionParametersContext* unambiguousFunctionParameters();

  class  UnambiguousFunctionParameterListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousFunctionParameterContext *> unambiguousFunctionParameter();
    UnambiguousFunctionParameterContext* unambiguousFunctionParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionParameterListContext* unambiguousFunctionParameterList();

  class  UnambiguousFunctionParameterContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *BYREF();
    antlr4::tree::TerminalNode *UNUSED();
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionParameterContext* unambiguousFunctionParameter();

  class  ScopedMethodCallContext : public antlr4::ParserRuleContext {
  public:
    ScopedMethodCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COLONCOLON();
    MethodCallContext *methodCall();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ScopedMethodCallContext* scopedMethodCall();

  class  UnambiguousExpressionContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MembershipContext *> membership();
    MembershipContext* membership(size_t i);
    std::vector<AssignmentOperatorContext *> assignmentOperator();
    AssignmentOperatorContext* assignmentOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExpressionContext* unambiguousExpression();

  class  AssignmentOperatorContext : public antlr4::ParserRuleContext {
  public:
    AssignmentOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *ADD_ASSIGN();
    antlr4::tree::TerminalNode *SUB_ASSIGN();
    antlr4::tree::TerminalNode *MUL_ASSIGN();
    antlr4::tree::TerminalNode *DIV_ASSIGN();
    antlr4::tree::TerminalNode *MOD_ASSIGN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentOperatorContext* assignmentOperator();

  class  MembershipContext : public antlr4::ParserRuleContext {
  public:
    MembershipContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<DisjunctionContext *> disjunction();
    DisjunctionContext* disjunction(size_t i);
    std::vector<MembershipOperatorContext *> membershipOperator();
    MembershipOperatorContext* membershipOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MembershipContext* membership();

  class  MembershipOperatorContext : public antlr4::ParserRuleContext {
  public:
    MembershipOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ADDMEMBER();
    antlr4::tree::TerminalNode *DELMEMBER();
    antlr4::tree::TerminalNode *CHKMEMBER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MembershipOperatorContext* membershipOperator();

  class  DisjunctionContext : public antlr4::ParserRuleContext {
  public:
    DisjunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ConjunctionContext *> conjunction();
    ConjunctionContext* conjunction(size_t i);
    std::vector<DisjunctionOperatorContext *> disjunctionOperator();
    DisjunctionOperatorContext* disjunctionOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DisjunctionContext* disjunction();

  class  DisjunctionOperatorContext : public antlr4::ParserRuleContext {
  public:
    DisjunctionOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OR_A();
    antlr4::tree::TerminalNode *OR_B();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DisjunctionOperatorContext* disjunctionOperator();

  class  ConjunctionContext : public antlr4::ParserRuleContext {
  public:
    ConjunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<EqualityContext *> equality();
    EqualityContext* equality(size_t i);
    std::vector<ConjunctionOperatorContext *> conjunctionOperator();
    ConjunctionOperatorContext* conjunctionOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConjunctionContext* conjunction();

  class  ConjunctionOperatorContext : public antlr4::ParserRuleContext {
  public:
    ConjunctionOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AND_A();
    antlr4::tree::TerminalNode *AND_B();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConjunctionOperatorContext* conjunctionOperator();

  class  EqualityContext : public antlr4::ParserRuleContext {
  public:
    EqualityContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitwiseDisjunctionContext *> bitwiseDisjunction();
    BitwiseDisjunctionContext* bitwiseDisjunction(size_t i);
    std::vector<EqualityOperatorContext *> equalityOperator();
    EqualityOperatorContext* equalityOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EqualityContext* equality();

  class  EqualityOperatorContext : public antlr4::ParserRuleContext {
  public:
    EqualityOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EQUAL();
    antlr4::tree::TerminalNode *EQUAL_DEPRECATED();
    antlr4::tree::TerminalNode *NOTEQUAL_A();
    antlr4::tree::TerminalNode *NOTEQUAL_B();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EqualityOperatorContext* equalityOperator();

  class  BitwiseDisjunctionContext : public antlr4::ParserRuleContext {
  public:
    BitwiseDisjunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitwiseXorContext *> bitwiseXor();
    BitwiseXorContext* bitwiseXor(size_t i);
    std::vector<antlr4::tree::TerminalNode *> BITOR();
    antlr4::tree::TerminalNode* BITOR(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseDisjunctionContext* bitwiseDisjunction();

  class  BitwiseXorContext : public antlr4::ParserRuleContext {
  public:
    BitwiseXorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitwiseConjunctionContext *> bitwiseConjunction();
    BitwiseConjunctionContext* bitwiseConjunction(size_t i);
    std::vector<antlr4::tree::TerminalNode *> CARET();
    antlr4::tree::TerminalNode* CARET(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseXorContext* bitwiseXor();

  class  BitwiseConjunctionContext : public antlr4::ParserRuleContext {
  public:
    BitwiseConjunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitshiftRightContext *> bitshiftRight();
    BitshiftRightContext* bitshiftRight(size_t i);
    std::vector<antlr4::tree::TerminalNode *> BITAND();
    antlr4::tree::TerminalNode* BITAND(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseConjunctionContext* bitwiseConjunction();

  class  BitshiftRightContext : public antlr4::ParserRuleContext {
  public:
    BitshiftRightContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitshiftLeftContext *> bitshiftLeft();
    BitshiftLeftContext* bitshiftLeft(size_t i);
    std::vector<antlr4::tree::TerminalNode *> RSHIFT();
    antlr4::tree::TerminalNode* RSHIFT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitshiftRightContext* bitshiftRight();

  class  BitshiftLeftContext : public antlr4::ParserRuleContext {
  public:
    BitshiftLeftContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ComparisonContext *> comparison();
    ComparisonContext* comparison(size_t i);
    std::vector<antlr4::tree::TerminalNode *> LSHIFT();
    antlr4::tree::TerminalNode* LSHIFT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitshiftLeftContext* bitshiftLeft();

  class  ComparisonContext : public antlr4::ParserRuleContext {
  public:
    ComparisonContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<InfixOperationContext *> infixOperation();
    InfixOperationContext* infixOperation(size_t i);
    std::vector<ComparisonOperatorContext *> comparisonOperator();
    ComparisonOperatorContext* comparisonOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComparisonContext* comparison();

  class  ComparisonOperatorContext : public antlr4::ParserRuleContext {
  public:
    ComparisonOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LE();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *GE();
    antlr4::tree::TerminalNode *GT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComparisonOperatorContext* comparisonOperator();

  class  InfixOperationContext : public antlr4::ParserRuleContext {
  public:
    InfixOperationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ElvisExpressionContext *> elvisExpression();
    ElvisExpressionContext* elvisExpression(size_t i);
    std::vector<InfixOperatorContext *> infixOperator();
    InfixOperatorContext* infixOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InfixOperationContext* infixOperation();

  class  InfixOperatorContext : public antlr4::ParserRuleContext {
  public:
    InfixOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_IN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InfixOperatorContext* infixOperator();

  class  ElvisExpressionContext : public antlr4::ParserRuleContext {
  public:
    ElvisExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AdditiveExpressionContext *> additiveExpression();
    AdditiveExpressionContext* additiveExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ELVIS();
    antlr4::tree::TerminalNode* ELVIS(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ElvisExpressionContext* elvisExpression();

  class  AdditiveExpressionContext : public antlr4::ParserRuleContext {
  public:
    AdditiveExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MultiplicativeExpressionContext *> multiplicativeExpression();
    MultiplicativeExpressionContext* multiplicativeExpression(size_t i);
    std::vector<AdditiveOperatorContext *> additiveOperator();
    AdditiveOperatorContext* additiveOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AdditiveExpressionContext* additiveExpression();

  class  AdditiveOperatorContext : public antlr4::ParserRuleContext {
  public:
    AdditiveOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ADD();
    antlr4::tree::TerminalNode *SUB();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AdditiveOperatorContext* additiveOperator();

  class  MultiplicativeExpressionContext : public antlr4::ParserRuleContext {
  public:
    MultiplicativeExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PrefixUnaryInversionExpressionContext *> prefixUnaryInversionExpression();
    PrefixUnaryInversionExpressionContext* prefixUnaryInversionExpression(size_t i);
    std::vector<MultiplicativeOperatorContext *> multiplicativeOperator();
    MultiplicativeOperatorContext* multiplicativeOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MultiplicativeExpressionContext* multiplicativeExpression();

  class  MultiplicativeOperatorContext : public antlr4::ParserRuleContext {
  public:
    MultiplicativeOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MUL();
    antlr4::tree::TerminalNode *DIV();
    antlr4::tree::TerminalNode *MOD();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MultiplicativeOperatorContext* multiplicativeOperator();

  class  PrefixUnaryInversionExpressionContext : public antlr4::ParserRuleContext {
  public:
    PrefixUnaryInversionExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrefixUnaryArithmeticExpressionContext *prefixUnaryArithmeticExpression();
    std::vector<PrefixUnaryInversionOperatorContext *> prefixUnaryInversionOperator();
    PrefixUnaryInversionOperatorContext* prefixUnaryInversionOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixUnaryInversionExpressionContext* prefixUnaryInversionExpression();

  class  PrefixUnaryInversionOperatorContext : public antlr4::ParserRuleContext {
  public:
    PrefixUnaryInversionOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TILDE();
    antlr4::tree::TerminalNode *BANG_A();
    antlr4::tree::TerminalNode *BANG_B();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixUnaryInversionOperatorContext* prefixUnaryInversionOperator();

  class  PrefixUnaryArithmeticExpressionContext : public antlr4::ParserRuleContext {
  public:
    PrefixUnaryArithmeticExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PostfixUnaryExpressionContext *postfixUnaryExpression();
    std::vector<PrefixUnaryArithmeticOperatorContext *> prefixUnaryArithmeticOperator();
    PrefixUnaryArithmeticOperatorContext* prefixUnaryArithmeticOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixUnaryArithmeticExpressionContext* prefixUnaryArithmeticExpression();

  class  PrefixUnaryArithmeticOperatorContext : public antlr4::ParserRuleContext {
  public:
    PrefixUnaryArithmeticOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INC();
    antlr4::tree::TerminalNode *DEC();
    antlr4::tree::TerminalNode *ADD();
    antlr4::tree::TerminalNode *SUB();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixUnaryArithmeticOperatorContext* prefixUnaryArithmeticOperator();

  class  PostfixUnaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    PostfixUnaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AtomicExpressionContext *atomicExpression();
    std::vector<PostfixUnaryOperatorContext *> postfixUnaryOperator();
    PostfixUnaryOperatorContext* postfixUnaryOperator(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PostfixUnaryExpressionContext* postfixUnaryExpression();

  class  PostfixUnaryOperatorContext : public antlr4::ParserRuleContext {
  public:
    PostfixUnaryOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INC();
    antlr4::tree::TerminalNode *DEC();
    IndexingSuffixContext *indexingSuffix();
    NavigationSuffixContext *navigationSuffix();
    MemberCallSuffixContext *memberCallSuffix();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PostfixUnaryOperatorContext* postfixUnaryOperator();

  class  IndexingSuffixContext : public antlr4::ParserRuleContext {
  public:
    IndexingSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACK();
    UnambiguousExpressionListContext *unambiguousExpressionList();
    antlr4::tree::TerminalNode *RBRACK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IndexingSuffixContext* indexingSuffix();

  class  NavigationSuffixContext : public antlr4::ParserRuleContext {
  public:
    NavigationSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MemberAccessOperatorContext *memberAccessOperator();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NavigationSuffixContext* navigationSuffix();

  class  MembershipSuffixContext : public antlr4::ParserRuleContext {
  public:
    MembershipSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MembershipOperatorContext *membershipOperator();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MembershipSuffixContext* membershipSuffix();

  class  MemberCallSuffixContext : public antlr4::ParserRuleContext {
  public:
    MemberCallSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MemberAccessOperatorContext *memberAccessOperator();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    UnambiguousExpressionListContext *unambiguousExpressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MemberCallSuffixContext* memberCallSuffix();

  class  MemberAccessOperatorContext : public antlr4::ParserRuleContext {
  public:
    MemberAccessOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MemberAccessOperatorContext* memberAccessOperator();

  class  CallSuffixContext : public antlr4::ParserRuleContext {
  public:
    CallSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueArgumentsContext *valueArguments();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallSuffixContext* callSuffix();

  class  AtomicExpressionContext : public antlr4::ParserRuleContext {
  public:
    AtomicExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LiteralContext *literal();
    ParenthesizedExpressionContext *parenthesizedExpression();
    UnambiguousFunctionCallContext *unambiguousFunctionCall();
    antlr4::tree::TerminalNode *IDENTIFIER();
    FunctionReferenceContext *functionReference();
    UnambiguousScopedFunctionCallContext *unambiguousScopedFunctionCall();
    UnambiguousExplicitArrayInitializerContext *unambiguousExplicitArrayInitializer();
    UnambiguousExplicitStructInitializerContext *unambiguousExplicitStructInitializer();
    UnambiguousExplicitDictInitializerContext *unambiguousExplicitDictInitializer();
    UnambiguousExplicitErrorInitializerContext *unambiguousExplicitErrorInitializer();
    UnambiguousBareArrayInitializerContext *unambiguousBareArrayInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AtomicExpressionContext* atomicExpression();

  class  FunctionReferenceContext : public antlr4::ParserRuleContext {
  public:
    FunctionReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionReferenceContext* functionReference();

  class  UnambiguousExplicitArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExplicitArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ARRAY();
    UnambiguousArrayInitializerContext *unambiguousArrayInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExplicitArrayInitializerContext* unambiguousExplicitArrayInitializer();

  class  UnambiguousExplicitStructInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExplicitStructInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRUCT();
    UnambiguousStructInitializerContext *unambiguousStructInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExplicitStructInitializerContext* unambiguousExplicitStructInitializer();

  class  UnambiguousExplicitDictInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExplicitDictInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DICTIONARY();
    UnambiguousDictInitializerContext *unambiguousDictInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExplicitDictInitializerContext* unambiguousExplicitDictInitializer();

  class  UnambiguousExplicitErrorInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExplicitErrorInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TOK_ERROR();
    UnambiguousStructInitializerContext *unambiguousStructInitializer();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExplicitErrorInitializerContext* unambiguousExplicitErrorInitializer();

  class  UnambiguousBareArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousBareArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    UnambiguousExpressionListContext *unambiguousExpressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousBareArrayInitializerContext* unambiguousBareArrayInitializer();

  class  ParenthesizedExpressionContext : public antlr4::ParserRuleContext {
  public:
    ParenthesizedExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    UnambiguousExpressionContext *unambiguousExpression();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParenthesizedExpressionContext* parenthesizedExpression();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *prefix = nullptr;;
    antlr4::Token *bop = nullptr;;
    antlr4::Token *postfix = nullptr;;
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryContext *primary();
    MethodCallContext *methodCall();
    ScopedMethodCallContext *scopedMethodCall();
    antlr4::tree::TerminalNode *ARRAY();
    ArrayInitializerContext *arrayInitializer();
    antlr4::tree::TerminalNode *STRUCT();
    StructInitializerContext *structInitializer();
    antlr4::tree::TerminalNode *DICTIONARY();
    DictInitializerContext *dictInitializer();
    antlr4::tree::TerminalNode *TOK_ERROR();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    ExpressionListContext *expressionList();
    FunctionReferenceContext *functionReference();
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
    antlr4::tree::TerminalNode *ELVIS();
    antlr4::tree::TerminalNode *LE();
    antlr4::tree::TerminalNode *GE();
    antlr4::tree::TerminalNode *GT();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *EQUAL();
    antlr4::tree::TerminalNode *EQUAL_DEPRECATED();
    antlr4::tree::TerminalNode *NOTEQUAL_B();
    antlr4::tree::TerminalNode *NOTEQUAL_A();
    antlr4::tree::TerminalNode *BITAND();
    antlr4::tree::TerminalNode *CARET();
    antlr4::tree::TerminalNode *BITOR();
    antlr4::tree::TerminalNode *TOK_IN();
    antlr4::tree::TerminalNode *AND_A();
    antlr4::tree::TerminalNode *AND_B();
    antlr4::tree::TerminalNode *OR_A();
    antlr4::tree::TerminalNode *OR_B();
    antlr4::tree::TerminalNode *ADDMEMBER();
    antlr4::tree::TerminalNode *DELMEMBER();
    antlr4::tree::TerminalNode *CHKMEMBER();
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *ADD_ASSIGN();
    antlr4::tree::TerminalNode *SUB_ASSIGN();
    antlr4::tree::TerminalNode *MUL_ASSIGN();
    antlr4::tree::TerminalNode *DIV_ASSIGN();
    antlr4::tree::TerminalNode *MOD_ASSIGN();
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    MemberCallContext *memberCall();
    antlr4::tree::TerminalNode *LBRACK();
    antlr4::tree::TerminalNode *RBRACK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  PrimaryContext : public antlr4::ParserRuleContext {
  public:
    PrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();
    LiteralContext *literal();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryContext* primary();

  class  ParExpressionContext : public antlr4::ParserRuleContext {
  public:
    ParExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParExpressionContext* parExpression();

  class  ExpressionListContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionListContext* expressionList();

  class  UnambiguousExpressionListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousExpressionContext *> unambiguousExpression();
    UnambiguousExpressionContext* unambiguousExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousExpressionListContext* unambiguousExpressionList();

  class  MethodCallArgumentContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *parameter = nullptr;;
    MethodCallArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *ASSIGN();
    antlr4::tree::TerminalNode *IDENTIFIER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MethodCallArgumentContext* methodCallArgument();

  class  MethodCallArgumentListContext : public antlr4::ParserRuleContext {
  public:
    MethodCallArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MethodCallArgumentContext *> methodCallArgument();
    MethodCallArgumentContext* methodCallArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MethodCallArgumentListContext* methodCallArgumentList();

  class  MethodCallContext : public antlr4::ParserRuleContext {
  public:
    MethodCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    MethodCallArgumentListContext *methodCallArgumentList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MethodCallContext* methodCall();

  class  ValueArgumentListContext : public antlr4::ParserRuleContext {
  public:
    ValueArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousFunctionCallArgumentContext *> unambiguousFunctionCallArgument();
    UnambiguousFunctionCallArgumentContext* unambiguousFunctionCallArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueArgumentListContext* valueArgumentList();

  class  ValueArgumentsContext : public antlr4::ParserRuleContext {
  public:
    ValueArgumentsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LPAREN();
    UnambiguousFunctionCallArgumentListContext *unambiguousFunctionCallArgumentList();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueArgumentsContext* valueArguments();

  class  UnambiguousFunctionCallArgumentContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionCallArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UnambiguousExpressionContext *unambiguousExpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionCallArgumentContext* unambiguousFunctionCallArgument();

  class  UnambiguousFunctionCallContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    ValueArgumentsContext *valueArguments();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionCallContext* unambiguousFunctionCall();

  class  UnambiguousScopedFunctionCallContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousScopedFunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *COLONCOLON();
    UnambiguousFunctionCallContext *unambiguousFunctionCall();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousScopedFunctionCallContext* unambiguousScopedFunctionCall();

  class  UnambiguousFunctionCallArgumentListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousFunctionCallArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousFunctionCallArgumentContext *> unambiguousFunctionCallArgument();
    UnambiguousFunctionCallArgumentContext* unambiguousFunctionCallArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousFunctionCallArgumentListContext* unambiguousFunctionCallArgumentList();

  class  MemberCallContext : public antlr4::ParserRuleContext {
  public:
    MemberCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ExpressionListContext *expressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MemberCallContext* memberCall();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructInitializerExpressionListContext* structInitializerExpressionList();

  class  StructInitializerContext : public antlr4::ParserRuleContext {
  public:
    StructInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    StructInitializerExpressionListContext *structInitializerExpressionList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructInitializerContext* structInitializer();

  class  UnambiguousStructInitializerExpressionContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousStructInitializerExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *ASSIGN();
    UnambiguousExpressionContext *unambiguousExpression();
    antlr4::tree::TerminalNode *STRING_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousStructInitializerExpressionContext* unambiguousStructInitializerExpression();

  class  UnambiguousStructInitializerExpressionListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousStructInitializerExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousStructInitializerExpressionContext *> unambiguousStructInitializerExpression();
    UnambiguousStructInitializerExpressionContext* unambiguousStructInitializerExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousStructInitializerExpressionListContext* unambiguousStructInitializerExpressionList();

  class  UnambiguousStructInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousStructInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    UnambiguousStructInitializerExpressionListContext *unambiguousStructInitializerExpressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousStructInitializerContext* unambiguousStructInitializer();

  class  DictInitializerExpressionContext : public antlr4::ParserRuleContext {
  public:
    DictInitializerExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *ARROW();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerExpressionContext* dictInitializerExpression();

  class  UnambiguousDictInitializerExpressionContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousDictInitializerExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousExpressionContext *> unambiguousExpression();
    UnambiguousExpressionContext* unambiguousExpression(size_t i);
    antlr4::tree::TerminalNode *ARROW();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousDictInitializerExpressionContext* unambiguousDictInitializerExpression();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerExpressionListContext* dictInitializerExpressionList();

  class  UnambiguousDictInitializerExpressionListContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousDictInitializerExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnambiguousDictInitializerExpressionContext *> unambiguousDictInitializerExpression();
    UnambiguousDictInitializerExpressionContext* unambiguousDictInitializerExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousDictInitializerExpressionListContext* unambiguousDictInitializerExpressionList();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DictInitializerContext* dictInitializer();

  class  UnambiguousDictInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousDictInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    UnambiguousDictInitializerExpressionListContext *unambiguousDictInitializerExpressionList();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousDictInitializerContext* unambiguousDictInitializer();

  class  ArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    ArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    ExpressionListContext *expressionList();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArrayInitializerContext* arrayInitializer();

  class  UnambiguousArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    UnambiguousArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    UnambiguousExpressionListContext *unambiguousExpressionList();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnambiguousArrayInitializerContext* unambiguousArrayInitializer();

  class  LiteralContext : public antlr4::ParserRuleContext {
  public:
    LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IntegerLiteralContext *integerLiteral();
    FloatLiteralContext *floatLiteral();
    antlr4::tree::TerminalNode *CHAR_LITERAL();
    antlr4::tree::TerminalNode *STRING_LITERAL();
    antlr4::tree::TerminalNode *NULL_LITERAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LiteralContext* literal();

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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FloatLiteralContext* floatLiteral();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace EscriptGrammar
