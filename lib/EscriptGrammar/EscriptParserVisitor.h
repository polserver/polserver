


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
    virtual antlrcpp::Any visitUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext *context) = 0;

    virtual antlrcpp::Any visitCompilationUnit(EscriptParser::CompilationUnitContext *context) = 0;

    virtual antlrcpp::Any visitModuleUnit(EscriptParser::ModuleUnitContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext *context) = 0;

    virtual antlrcpp::Any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitStringIdentifier(EscriptParser::StringIdentifierContext *context) = 0;

    virtual antlrcpp::Any visitUseDeclaration(EscriptParser::UseDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitProgramDeclaration(EscriptParser::ProgramDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousStatement(EscriptParser::UnambiguousStatementContext *context) = 0;

    virtual antlrcpp::Any visitStatement(EscriptParser::StatementContext *context) = 0;

    virtual antlrcpp::Any visitStatementLabel(EscriptParser::StatementLabelContext *context) = 0;

    virtual antlrcpp::Any visitIfStatement(EscriptParser::IfStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext *context) = 0;

    virtual antlrcpp::Any visitGotoStatement(EscriptParser::GotoStatementContext *context) = 0;

    virtual antlrcpp::Any visitReturnStatement(EscriptParser::ReturnStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext *context) = 0;

    virtual antlrcpp::Any visitConstStatement(EscriptParser::ConstStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext *context) = 0;

    virtual antlrcpp::Any visitVarStatement(EscriptParser::VarStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext *context) = 0;

    virtual antlrcpp::Any visitDoStatement(EscriptParser::DoStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext *context) = 0;

    virtual antlrcpp::Any visitWhileStatement(EscriptParser::WhileStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext *context) = 0;

    virtual antlrcpp::Any visitExitStatement(EscriptParser::ExitStatementContext *context) = 0;

    virtual antlrcpp::Any visitDeclareStatement(EscriptParser::DeclareStatementContext *context) = 0;

    virtual antlrcpp::Any visitBreakStatement(EscriptParser::BreakStatementContext *context) = 0;

    virtual antlrcpp::Any visitContinueStatement(EscriptParser::ContinueStatementContext *context) = 0;

    virtual antlrcpp::Any visitForStatement(EscriptParser::ForStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext *context) = 0;

    virtual antlrcpp::Any visitForeachStatement(EscriptParser::ForeachStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext *context) = 0;

    virtual antlrcpp::Any visitRepeatStatement(EscriptParser::RepeatStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext *context) = 0;

    virtual antlrcpp::Any visitCaseStatement(EscriptParser::CaseStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext *context) = 0;

    virtual antlrcpp::Any visitEnumStatement(EscriptParser::EnumStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext *context) = 0;

    virtual antlrcpp::Any visitBlock(EscriptParser::BlockContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousBlock(EscriptParser::UnambiguousBlockContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext *context) = 0;

    virtual antlrcpp::Any visitEnumList(EscriptParser::EnumListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext *context) = 0;

    virtual antlrcpp::Any visitEnumListEntry(EscriptParser::EnumListEntryContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext *context) = 0;

    virtual antlrcpp::Any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext *context) = 0;

    virtual antlrcpp::Any visitSwitchLabel(EscriptParser::SwitchLabelContext *context) = 0;

    virtual antlrcpp::Any visitForGroup(EscriptParser::ForGroupContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext *context) = 0;

    virtual antlrcpp::Any visitBasicForStatement(EscriptParser::BasicForStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext *context) = 0;

    virtual antlrcpp::Any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext *context) = 0;

    virtual antlrcpp::Any visitIdentifierList(EscriptParser::IdentifierListContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameters(EscriptParser::ProgramParametersContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameterList(EscriptParser::ProgramParameterListContext *context) = 0;

    virtual antlrcpp::Any visitProgramParameter(EscriptParser::ProgramParameterContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameters(EscriptParser::FunctionParametersContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitFunctionParameter(EscriptParser::FunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext *context) = 0;

    virtual antlrcpp::Any visitScopedMethodCall(EscriptParser::ScopedMethodCallContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExpression(EscriptParser::UnambiguousExpressionContext *context) = 0;

    virtual antlrcpp::Any visitAssignmentOperator(EscriptParser::AssignmentOperatorContext *context) = 0;

    virtual antlrcpp::Any visitMembership(EscriptParser::MembershipContext *context) = 0;

    virtual antlrcpp::Any visitMembershipOperator(EscriptParser::MembershipOperatorContext *context) = 0;

    virtual antlrcpp::Any visitDisjunction(EscriptParser::DisjunctionContext *context) = 0;

    virtual antlrcpp::Any visitDisjunctionOperator(EscriptParser::DisjunctionOperatorContext *context) = 0;

    virtual antlrcpp::Any visitConjunction(EscriptParser::ConjunctionContext *context) = 0;

    virtual antlrcpp::Any visitConjunctionOperator(EscriptParser::ConjunctionOperatorContext *context) = 0;

    virtual antlrcpp::Any visitEquality(EscriptParser::EqualityContext *context) = 0;

    virtual antlrcpp::Any visitEqualityOperator(EscriptParser::EqualityOperatorContext *context) = 0;

    virtual antlrcpp::Any visitBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext *context) = 0;

    virtual antlrcpp::Any visitBitwiseXor(EscriptParser::BitwiseXorContext *context) = 0;

    virtual antlrcpp::Any visitBitwiseConjunction(EscriptParser::BitwiseConjunctionContext *context) = 0;

    virtual antlrcpp::Any visitBitshiftRight(EscriptParser::BitshiftRightContext *context) = 0;

    virtual antlrcpp::Any visitBitshiftLeft(EscriptParser::BitshiftLeftContext *context) = 0;

    virtual antlrcpp::Any visitComparison(EscriptParser::ComparisonContext *context) = 0;

    virtual antlrcpp::Any visitComparisonOperator(EscriptParser::ComparisonOperatorContext *context) = 0;

    virtual antlrcpp::Any visitInfixOperation(EscriptParser::InfixOperationContext *context) = 0;

    virtual antlrcpp::Any visitInfixOperator(EscriptParser::InfixOperatorContext *context) = 0;

    virtual antlrcpp::Any visitElvisExpression(EscriptParser::ElvisExpressionContext *context) = 0;

    virtual antlrcpp::Any visitAdditiveExpression(EscriptParser::AdditiveExpressionContext *context) = 0;

    virtual antlrcpp::Any visitAdditiveOperator(EscriptParser::AdditiveOperatorContext *context) = 0;

    virtual antlrcpp::Any visitMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext *context) = 0;

    virtual antlrcpp::Any visitMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext *context) = 0;

    virtual antlrcpp::Any visitPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext *context) = 0;

    virtual antlrcpp::Any visitPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext *context) = 0;

    virtual antlrcpp::Any visitPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext *context) = 0;

    virtual antlrcpp::Any visitPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext *context) = 0;

    virtual antlrcpp::Any visitPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext *context) = 0;

    virtual antlrcpp::Any visitPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext *context) = 0;

    virtual antlrcpp::Any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *context) = 0;

    virtual antlrcpp::Any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *context) = 0;

    virtual antlrcpp::Any visitMembershipSuffix(EscriptParser::MembershipSuffixContext *context) = 0;

    virtual antlrcpp::Any visitMemberCallSuffix(EscriptParser::MemberCallSuffixContext *context) = 0;

    virtual antlrcpp::Any visitMemberAccessOperator(EscriptParser::MemberAccessOperatorContext *context) = 0;

    virtual antlrcpp::Any visitCallSuffix(EscriptParser::CallSuffixContext *context) = 0;

    virtual antlrcpp::Any visitAtomicExpression(EscriptParser::AtomicExpressionContext *context) = 0;

    virtual antlrcpp::Any visitFunctionReference(EscriptParser::FunctionReferenceContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext *context) = 0;

    virtual antlrcpp::Any visitExpression(EscriptParser::ExpressionContext *context) = 0;

    virtual antlrcpp::Any visitPrimary(EscriptParser::PrimaryContext *context) = 0;

    virtual antlrcpp::Any visitParExpression(EscriptParser::ParExpressionContext *context) = 0;

    virtual antlrcpp::Any visitExpressionList(EscriptParser::ExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitMethodCallArgument(EscriptParser::MethodCallArgumentContext *context) = 0;

    virtual antlrcpp::Any visitMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext *context) = 0;

    virtual antlrcpp::Any visitMethodCall(EscriptParser::MethodCallContext *context) = 0;

    virtual antlrcpp::Any visitValueArgumentList(EscriptParser::ValueArgumentListContext *context) = 0;

    virtual antlrcpp::Any visitValueArguments(EscriptParser::ValueArgumentsContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext *context) = 0;

    virtual antlrcpp::Any visitMemberCall(EscriptParser::MemberCallContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitStructInitializer(EscriptParser::StructInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitDictInitializer(EscriptParser::DictInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext *context) = 0;

    virtual antlrcpp::Any visitArrayInitializer(EscriptParser::ArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext *context) = 0;

    virtual antlrcpp::Any visitLiteral(EscriptParser::LiteralContext *context) = 0;

    virtual antlrcpp::Any visitIntegerLiteral(EscriptParser::IntegerLiteralContext *context) = 0;

    virtual antlrcpp::Any visitFloatLiteral(EscriptParser::FloatLiteralContext *context) = 0;


};

}  // namespace EscriptGrammar
