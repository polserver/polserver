
#include "EscriptParserRuleContext.h"


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

  virtual antlrcpp::Any visitCompilationUnit(EscriptParser::CompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleUnit(EscriptParser::ModuleUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) override {
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

  virtual antlrcpp::Any visitStatement(EscriptParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatementLabel(EscriptParser::StatementLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfStatement(EscriptParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGotoStatement(EscriptParser::GotoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturnStatement(EscriptParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstStatement(EscriptParser::ConstStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarStatement(EscriptParser::VarStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDoStatement(EscriptParser::DoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhileStatement(EscriptParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExitStatement(EscriptParser::ExitStatementContext *ctx) override {
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

  virtual antlrcpp::Any visitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForeachStatement(EscriptParser::ForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRepeatStatement(EscriptParser::RepeatStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCaseStatement(EscriptParser::CaseStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumStatement(EscriptParser::EnumStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlock(EscriptParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumList(EscriptParser::EnumListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumListEntry(EscriptParser::EnumListEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSwitchLabel(EscriptParser::SwitchLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForGroup(EscriptParser::ForGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBasicForStatement(EscriptParser::BasicForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifierList(EscriptParser::IdentifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstantDeclaration(EscriptParser::ConstantDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) override {
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

  virtual antlrcpp::Any visitFunctionParameters(EscriptParser::FunctionParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionParameter(EscriptParser::FunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionReference(EscriptParser::FunctionReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpression(EscriptParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimary(EscriptParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBareArrayInitializer(EscriptParser::BareArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParExpression(EscriptParser::ParExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionList(EscriptParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionSuffix(EscriptParser::ExpressionSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMethodCallSuffix(EscriptParser::MethodCallSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionCall(EscriptParser::FunctionCallContext *ctx) override {
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

  virtual antlrcpp::Any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDictInitializer(EscriptParser::DictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteral(EscriptParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInterpolatedString(EscriptParser::InterpolatedStringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *ctx) override {
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
