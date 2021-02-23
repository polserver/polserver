


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "EscriptParser.h"


namespace EscriptGrammar {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by EscriptParser.
 */
class  EscriptParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by EscriptParser.
   */
    virtual antlrcpp::Any visitCompilationUnit(EscriptParser::CompilationUnitContext *context) = 0;

    virtual antlrcpp::Any visitModuleUnit(EscriptParser::ModuleUnitContext *context) = 0;

    virtual antlrcpp::Any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitStringIdentifier(EscriptParser::StringIdentifierContext *context) = 0;

    virtual antlrcpp::Any visitUseDeclaration(EscriptParser::UseDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitProgramDeclaration(EscriptParser::ProgramDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitStatement(EscriptParser::StatementContext *context) = 0;

    virtual antlrcpp::Any visitStatementLabel(EscriptParser::StatementLabelContext *context) = 0;

    virtual antlrcpp::Any visitIfStatement(EscriptParser::IfStatementContext *context) = 0;

    virtual antlrcpp::Any visitGotoStatement(EscriptParser::GotoStatementContext *context) = 0;

    virtual antlrcpp::Any visitReturnStatement(EscriptParser::ReturnStatementContext *context) = 0;

    virtual antlrcpp::Any visitConstStatement(EscriptParser::ConstStatementContext *context) = 0;

    virtual antlrcpp::Any visitVarStatement(EscriptParser::VarStatementContext *context) = 0;

    virtual antlrcpp::Any visitDoStatement(EscriptParser::DoStatementContext *context) = 0;

    virtual antlrcpp::Any visitWhileStatement(EscriptParser::WhileStatementContext *context) = 0;

    virtual antlrcpp::Any visitExitStatement(EscriptParser::ExitStatementContext *context) = 0;

    virtual antlrcpp::Any visitBreakStatement(EscriptParser::BreakStatementContext *context) = 0;

    virtual antlrcpp::Any visitContinueStatement(EscriptParser::ContinueStatementContext *context) = 0;

    virtual antlrcpp::Any visitForStatement(EscriptParser::ForStatementContext *context) = 0;

    virtual antlrcpp::Any visitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *context) = 0;

    virtual antlrcpp::Any visitForeachStatement(EscriptParser::ForeachStatementContext *context) = 0;

    virtual antlrcpp::Any visitRepeatStatement(EscriptParser::RepeatStatementContext *context) = 0;

    virtual antlrcpp::Any visitCaseStatement(EscriptParser::CaseStatementContext *context) = 0;

    virtual antlrcpp::Any visitEnumStatement(EscriptParser::EnumStatementContext *context) = 0;

    virtual antlrcpp::Any visitBlock(EscriptParser::BlockContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *context) = 0;

    virtual antlrcpp::Any visitEnumList(EscriptParser::EnumListContext *context) = 0;

    virtual antlrcpp::Any visitEnumListEntry(EscriptParser::EnumListEntryContext *context) = 0;

    virtual antlrcpp::Any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *context) = 0;

    virtual antlrcpp::Any visitSwitchLabel(EscriptParser::SwitchLabelContext *context) = 0;

    virtual antlrcpp::Any visitForGroup(EscriptParser::ForGroupContext *context) = 0;

    virtual antlrcpp::Any visitBasicForStatement(EscriptParser::BasicForStatementContext *context) = 0;

    virtual antlrcpp::Any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *context) = 0;

    virtual antlrcpp::Any visitIdentifierList(EscriptParser::IdentifierListContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameters(EscriptParser::ProgramParametersContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameterList(EscriptParser::ProgramParameterListContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameter(EscriptParser::ProgramParameterContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameters(EscriptParser::FunctionParametersContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameter(EscriptParser::FunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitFunctionReference(EscriptParser::FunctionReferenceContext *context) = 0;

    virtual antlrcpp::Any visitExpression(EscriptParser::ExpressionContext *context) = 0;

    virtual antlrcpp::Any visitPrimary(EscriptParser::PrimaryContext *context) = 0;

    virtual antlrcpp::Any visitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *context) = 0;

    virtual antlrcpp::Any visitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *context) = 0;

    virtual antlrcpp::Any visitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *context) = 0;

    virtual antlrcpp::Any visitBareArrayInitializer(EscriptParser::BareArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitParExpression(EscriptParser::ParExpressionContext *context) = 0;

    virtual antlrcpp::Any visitExpressionList(EscriptParser::ExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitExpressionSuffix(EscriptParser::ExpressionSuffixContext *context) = 0;

    virtual antlrcpp::Any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *context) = 0;

    virtual antlrcpp::Any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *context) = 0;

    virtual antlrcpp::Any visitMethodCallSuffix(EscriptParser::MethodCallSuffixContext *context) = 0;

    virtual antlrcpp::Any visitFunctionCall(EscriptParser::FunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializer(EscriptParser::StructInitializerContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializer(EscriptParser::DictInitializerContext *context) = 0;

    virtual antlrcpp::Any visitArrayInitializer(EscriptParser::ArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitLiteral(EscriptParser::LiteralContext *context) = 0;

    virtual antlrcpp::Any visitInterpolatedString(EscriptParser::InterpolatedStringContext *context) = 0;

    virtual antlrcpp::Any visitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *context) = 0;

    virtual antlrcpp::Any visitInterpolatedStringExpression(EscriptParser::InterpolatedStringExpressionContext *context) = 0;

    virtual antlrcpp::Any visitIntegerLiteral(EscriptParser::IntegerLiteralContext *context) = 0;

    virtual antlrcpp::Any visitFloatLiteral(EscriptParser::FloatLiteralContext *context) = 0;


};

}  // namespace EscriptGrammar
