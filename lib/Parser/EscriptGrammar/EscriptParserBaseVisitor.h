


// Generated from EscriptParser.g4 by ANTLR 4.13.1

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

  virtual std::any visitCompilationUnit(EscriptParser::CompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModuleUnit(EscriptParser::ModuleUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEvaluateUnit(EscriptParser::EvaluateUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassDeclaration(EscriptParser::ClassDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassParameters(EscriptParser::ClassParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassParameterList(EscriptParser::ClassParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassBody(EscriptParser::ClassBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassStatement(EscriptParser::ClassStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStringIdentifier(EscriptParser::StringIdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUseDeclaration(EscriptParser::UseDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(EscriptParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatementLabel(EscriptParser::StatementLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStatement(EscriptParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGotoStatement(EscriptParser::GotoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStatement(EscriptParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstStatement(EscriptParser::ConstStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarStatement(EscriptParser::VarStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDoStatement(EscriptParser::DoStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStatement(EscriptParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExitStatement(EscriptParser::ExitStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStatement(EscriptParser::BreakStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStatement(EscriptParser::ContinueStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForStatement(EscriptParser::ForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeachStatement(EscriptParser::ForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRepeatStatement(EscriptParser::RepeatStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCaseStatement(EscriptParser::CaseStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumStatement(EscriptParser::EnumStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(EscriptParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumList(EscriptParser::EnumListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumListEntry(EscriptParser::EnumListEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwitchLabel(EscriptParser::SwitchLabelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForGroup(EscriptParser::ForGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBasicForStatement(EscriptParser::BasicForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifierList(EscriptParser::IdentifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstantDeclaration(EscriptParser::ConstantDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnpackingDeclaration(EscriptParser::UnpackingDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberUnpackingList(EscriptParser::MemberUnpackingListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexUnpackingList(EscriptParser::IndexUnpackingListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexUnpacking(EscriptParser::IndexUnpackingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberUnpacking(EscriptParser::MemberUnpackingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnpacking(EscriptParser::UnpackingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnpackingDeclarationInitializer(EscriptParser::UnpackingDeclarationInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgramParameters(EscriptParser::ProgramParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgramParameter(EscriptParser::ProgramParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionParameters(EscriptParser::FunctionParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionParameter(EscriptParser::FunctionParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionReference(EscriptParser::FunctionReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(EscriptParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimary(EscriptParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScopedIdentifier(EscriptParser::ScopedIdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionExpression(EscriptParser::FunctionExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBareArrayInitializer(EscriptParser::BareArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParExpression(EscriptParser::ParExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionList(EscriptParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionListEntry(EscriptParser::ExpressionListEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionSuffix(EscriptParser::ExpressionSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexList(EscriptParser::IndexListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMethodCallSuffix(EscriptParser::MethodCallSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCallSuffix(EscriptParser::FunctionCallSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCall(EscriptParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructInitializer(EscriptParser::StructInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDictInitializer(EscriptParser::DictInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(EscriptParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInterpolatedString(EscriptParser::InterpolatedStringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFloatLiteral(EscriptParser::FloatLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolLiteral(EscriptParser::BoolLiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace EscriptGrammar
