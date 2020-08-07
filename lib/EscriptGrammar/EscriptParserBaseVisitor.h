


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "EscriptParserVisitor.h"


namespace EscriptGrammar {

/**
 * This class provides an empty implementation of EscriptParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  EscriptParserBaseVisitor : public EscriptParserVisitor {
public:

  virtual antlrcpp::Any visitUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompilationUnit(EscriptParser::CompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleUnit(EscriptParser::ModuleUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringIdentifier(EscriptParser::StringIdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUseDeclaration(EscriptParser::UseDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousStatement(EscriptParser::UnambiguousStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatement(EscriptParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatementLabel(EscriptParser::StatementLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfStatement(EscriptParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGotoStatement(EscriptParser::GotoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturnStatement(EscriptParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstStatement(EscriptParser::ConstStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarStatement(EscriptParser::VarStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDoStatement(EscriptParser::DoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhileStatement(EscriptParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExitStatement(EscriptParser::ExitStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDeclareStatement(EscriptParser::DeclareStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBreakStatement(EscriptParser::BreakStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContinueStatement(EscriptParser::ContinueStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForStatement(EscriptParser::ForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForeachStatement(EscriptParser::ForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRepeatStatement(EscriptParser::RepeatStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCaseStatement(EscriptParser::CaseStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumStatement(EscriptParser::EnumStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlock(EscriptParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousBlock(EscriptParser::UnambiguousBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumList(EscriptParser::EnumListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumListEntry(EscriptParser::EnumListEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSwitchLabel(EscriptParser::SwitchLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForGroup(EscriptParser::ForGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBasicForStatement(EscriptParser::BasicForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifierList(EscriptParser::IdentifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitProgramParameters(EscriptParser::ProgramParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitProgramParameter(EscriptParser::ProgramParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionParameters(EscriptParser::FunctionParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionParameter(EscriptParser::FunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitScopedMethodCall(EscriptParser::ScopedMethodCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExpression(EscriptParser::UnambiguousExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignmentOperator(EscriptParser::AssignmentOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMembership(EscriptParser::MembershipContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMembershipOperator(EscriptParser::MembershipOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDisjunction(EscriptParser::DisjunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDisjunctionOperator(EscriptParser::DisjunctionOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConjunction(EscriptParser::ConjunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConjunctionOperator(EscriptParser::ConjunctionOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEquality(EscriptParser::EqualityContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEqualityOperator(EscriptParser::EqualityOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitwiseXor(EscriptParser::BitwiseXorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitwiseConjunction(EscriptParser::BitwiseConjunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitshiftRight(EscriptParser::BitshiftRightContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitshiftLeft(EscriptParser::BitshiftLeftContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComparison(EscriptParser::ComparisonContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComparisonOperator(EscriptParser::ComparisonOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInfixOperation(EscriptParser::InfixOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInfixOperator(EscriptParser::InfixOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitElvisExpression(EscriptParser::ElvisExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAdditiveExpression(EscriptParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAdditiveOperator(EscriptParser::AdditiveOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMembershipSuffix(EscriptParser::MembershipSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMemberCallSuffix(EscriptParser::MemberCallSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMemberAccessOperator(EscriptParser::MemberAccessOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCallSuffix(EscriptParser::CallSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAtomicExpression(EscriptParser::AtomicExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionReference(EscriptParser::FunctionReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpression(EscriptParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimary(EscriptParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParExpression(EscriptParser::ParExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionList(EscriptParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMethodCallArgument(EscriptParser::MethodCallArgumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMethodCall(EscriptParser::MethodCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValueArgumentList(EscriptParser::ValueArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValueArguments(EscriptParser::ValueArgumentsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMemberCall(EscriptParser::MemberCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStructInitializer(EscriptParser::StructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDictInitializer(EscriptParser::DictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteral(EscriptParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFloatLiteral(EscriptParser::FloatLiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace EscriptGrammar
