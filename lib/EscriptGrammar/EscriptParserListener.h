


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "EscriptParser.h"


namespace EscriptGrammar {

/**
 * This interface defines an abstract listener for a parse tree produced by EscriptParser.
 */
class  EscriptParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext *ctx) = 0;
  virtual void exitUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext *ctx) = 0;

  virtual void enterCompilationUnit(EscriptParser::CompilationUnitContext *ctx) = 0;
  virtual void exitCompilationUnit(EscriptParser::CompilationUnitContext *ctx) = 0;

  virtual void enterModuleUnit(EscriptParser::ModuleUnitContext *ctx) = 0;
  virtual void exitModuleUnit(EscriptParser::ModuleUnitContext *ctx) = 0;

  virtual void enterUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext *ctx) = 0;
  virtual void exitUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext *ctx) = 0;

  virtual void enterModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) = 0;
  virtual void exitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) = 0;

  virtual void enterUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext *ctx) = 0;
  virtual void exitUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext *ctx) = 0;

  virtual void enterModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) = 0;
  virtual void exitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext *ctx) = 0;
  virtual void exitUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext *ctx) = 0;

  virtual void enterModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) = 0;
  virtual void exitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) = 0;

  virtual void enterUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext *ctx) = 0;
  virtual void exitUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext *ctx) = 0;

  virtual void enterModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) = 0;
  virtual void exitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) = 0;

  virtual void enterUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext *ctx) = 0;
  virtual void exitUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext *ctx) = 0;

  virtual void enterTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) = 0;
  virtual void exitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext *ctx) = 0;
  virtual void exitUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext *ctx) = 0;

  virtual void enterFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) = 0;
  virtual void exitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext *ctx) = 0;
  virtual void exitUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext *ctx) = 0;

  virtual void enterStringIdentifier(EscriptParser::StringIdentifierContext *ctx) = 0;
  virtual void exitStringIdentifier(EscriptParser::StringIdentifierContext *ctx) = 0;

  virtual void enterUseDeclaration(EscriptParser::UseDeclarationContext *ctx) = 0;
  virtual void exitUseDeclaration(EscriptParser::UseDeclarationContext *ctx) = 0;

  virtual void enterIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) = 0;
  virtual void exitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) = 0;

  virtual void enterProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) = 0;
  virtual void exitProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext *ctx) = 0;
  virtual void exitUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousStatement(EscriptParser::UnambiguousStatementContext *ctx) = 0;
  virtual void exitUnambiguousStatement(EscriptParser::UnambiguousStatementContext *ctx) = 0;

  virtual void enterStatement(EscriptParser::StatementContext *ctx) = 0;
  virtual void exitStatement(EscriptParser::StatementContext *ctx) = 0;

  virtual void enterStatementLabel(EscriptParser::StatementLabelContext *ctx) = 0;
  virtual void exitStatementLabel(EscriptParser::StatementLabelContext *ctx) = 0;

  virtual void enterIfStatement(EscriptParser::IfStatementContext *ctx) = 0;
  virtual void exitIfStatement(EscriptParser::IfStatementContext *ctx) = 0;

  virtual void enterUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext *ctx) = 0;
  virtual void exitUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext *ctx) = 0;

  virtual void enterGotoStatement(EscriptParser::GotoStatementContext *ctx) = 0;
  virtual void exitGotoStatement(EscriptParser::GotoStatementContext *ctx) = 0;

  virtual void enterReturnStatement(EscriptParser::ReturnStatementContext *ctx) = 0;
  virtual void exitReturnStatement(EscriptParser::ReturnStatementContext *ctx) = 0;

  virtual void enterUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext *ctx) = 0;
  virtual void exitUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext *ctx) = 0;

  virtual void enterConstStatement(EscriptParser::ConstStatementContext *ctx) = 0;
  virtual void exitConstStatement(EscriptParser::ConstStatementContext *ctx) = 0;

  virtual void enterUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext *ctx) = 0;
  virtual void exitUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext *ctx) = 0;

  virtual void enterVarStatement(EscriptParser::VarStatementContext *ctx) = 0;
  virtual void exitVarStatement(EscriptParser::VarStatementContext *ctx) = 0;

  virtual void enterUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext *ctx) = 0;
  virtual void exitUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext *ctx) = 0;

  virtual void enterDoStatement(EscriptParser::DoStatementContext *ctx) = 0;
  virtual void exitDoStatement(EscriptParser::DoStatementContext *ctx) = 0;

  virtual void enterUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext *ctx) = 0;
  virtual void exitUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext *ctx) = 0;

  virtual void enterWhileStatement(EscriptParser::WhileStatementContext *ctx) = 0;
  virtual void exitWhileStatement(EscriptParser::WhileStatementContext *ctx) = 0;

  virtual void enterUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext *ctx) = 0;
  virtual void exitUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext *ctx) = 0;

  virtual void enterExitStatement(EscriptParser::ExitStatementContext *ctx) = 0;
  virtual void exitExitStatement(EscriptParser::ExitStatementContext *ctx) = 0;

  virtual void enterDeclareStatement(EscriptParser::DeclareStatementContext *ctx) = 0;
  virtual void exitDeclareStatement(EscriptParser::DeclareStatementContext *ctx) = 0;

  virtual void enterBreakStatement(EscriptParser::BreakStatementContext *ctx) = 0;
  virtual void exitBreakStatement(EscriptParser::BreakStatementContext *ctx) = 0;

  virtual void enterContinueStatement(EscriptParser::ContinueStatementContext *ctx) = 0;
  virtual void exitContinueStatement(EscriptParser::ContinueStatementContext *ctx) = 0;

  virtual void enterForStatement(EscriptParser::ForStatementContext *ctx) = 0;
  virtual void exitForStatement(EscriptParser::ForStatementContext *ctx) = 0;

  virtual void enterUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext *ctx) = 0;
  virtual void exitUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext *ctx) = 0;

  virtual void enterForeachStatement(EscriptParser::ForeachStatementContext *ctx) = 0;
  virtual void exitForeachStatement(EscriptParser::ForeachStatementContext *ctx) = 0;

  virtual void enterUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext *ctx) = 0;
  virtual void exitUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext *ctx) = 0;

  virtual void enterRepeatStatement(EscriptParser::RepeatStatementContext *ctx) = 0;
  virtual void exitRepeatStatement(EscriptParser::RepeatStatementContext *ctx) = 0;

  virtual void enterUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext *ctx) = 0;
  virtual void exitUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext *ctx) = 0;

  virtual void enterCaseStatement(EscriptParser::CaseStatementContext *ctx) = 0;
  virtual void exitCaseStatement(EscriptParser::CaseStatementContext *ctx) = 0;

  virtual void enterUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext *ctx) = 0;
  virtual void exitUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext *ctx) = 0;

  virtual void enterEnumStatement(EscriptParser::EnumStatementContext *ctx) = 0;
  virtual void exitEnumStatement(EscriptParser::EnumStatementContext *ctx) = 0;

  virtual void enterUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext *ctx) = 0;
  virtual void exitUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext *ctx) = 0;

  virtual void enterBlock(EscriptParser::BlockContext *ctx) = 0;
  virtual void exitBlock(EscriptParser::BlockContext *ctx) = 0;

  virtual void enterUnambiguousBlock(EscriptParser::UnambiguousBlockContext *ctx) = 0;
  virtual void exitUnambiguousBlock(EscriptParser::UnambiguousBlockContext *ctx) = 0;

  virtual void enterVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) = 0;
  virtual void exitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) = 0;

  virtual void enterUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext *ctx) = 0;
  virtual void exitUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext *ctx) = 0;

  virtual void enterEnumList(EscriptParser::EnumListContext *ctx) = 0;
  virtual void exitEnumList(EscriptParser::EnumListContext *ctx) = 0;

  virtual void enterUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext *ctx) = 0;
  virtual void exitUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext *ctx) = 0;

  virtual void enterEnumListEntry(EscriptParser::EnumListEntryContext *ctx) = 0;
  virtual void exitEnumListEntry(EscriptParser::EnumListEntryContext *ctx) = 0;

  virtual void enterUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext *ctx) = 0;
  virtual void exitUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext *ctx) = 0;

  virtual void enterSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) = 0;
  virtual void exitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) = 0;

  virtual void enterUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext *ctx) = 0;
  virtual void exitUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext *ctx) = 0;

  virtual void enterSwitchLabel(EscriptParser::SwitchLabelContext *ctx) = 0;
  virtual void exitSwitchLabel(EscriptParser::SwitchLabelContext *ctx) = 0;

  virtual void enterForGroup(EscriptParser::ForGroupContext *ctx) = 0;
  virtual void exitForGroup(EscriptParser::ForGroupContext *ctx) = 0;

  virtual void enterUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext *ctx) = 0;
  virtual void exitUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext *ctx) = 0;

  virtual void enterBasicForStatement(EscriptParser::BasicForStatementContext *ctx) = 0;
  virtual void exitBasicForStatement(EscriptParser::BasicForStatementContext *ctx) = 0;

  virtual void enterUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext *ctx) = 0;
  virtual void exitUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext *ctx) = 0;

  virtual void enterCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) = 0;
  virtual void exitCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) = 0;

  virtual void enterUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext *ctx) = 0;
  virtual void exitUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext *ctx) = 0;

  virtual void enterIdentifierList(EscriptParser::IdentifierListContext *ctx) = 0;
  virtual void exitIdentifierList(EscriptParser::IdentifierListContext *ctx) = 0;

  virtual void enterVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) = 0;
  virtual void exitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) = 0;

  virtual void enterUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext *ctx) = 0;
  virtual void exitUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext *ctx) = 0;

  virtual void enterVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) = 0;
  virtual void exitVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) = 0;

  virtual void enterUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext *ctx) = 0;
  virtual void exitUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext *ctx) = 0;

  virtual void enterProgramParameters(EscriptParser::ProgramParametersContext *ctx) = 0;
  virtual void exitProgramParameters(EscriptParser::ProgramParametersContext *ctx) = 0;

  virtual void enterProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) = 0;
  virtual void exitProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) = 0;

  virtual void enterProgramParameter(EscriptParser::ProgramParameterContext *ctx) = 0;
  virtual void exitProgramParameter(EscriptParser::ProgramParameterContext *ctx) = 0;

  virtual void enterUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext *ctx) = 0;
  virtual void exitUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext *ctx) = 0;

  virtual void enterUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext *ctx) = 0;
  virtual void exitUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext *ctx) = 0;

  virtual void enterUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext *ctx) = 0;
  virtual void exitUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext *ctx) = 0;

  virtual void enterFunctionParameters(EscriptParser::FunctionParametersContext *ctx) = 0;
  virtual void exitFunctionParameters(EscriptParser::FunctionParametersContext *ctx) = 0;

  virtual void enterFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) = 0;
  virtual void exitFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) = 0;

  virtual void enterFunctionParameter(EscriptParser::FunctionParameterContext *ctx) = 0;
  virtual void exitFunctionParameter(EscriptParser::FunctionParameterContext *ctx) = 0;

  virtual void enterUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext *ctx) = 0;
  virtual void exitUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext *ctx) = 0;

  virtual void enterUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext *ctx) = 0;
  virtual void exitUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext *ctx) = 0;

  virtual void enterUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext *ctx) = 0;
  virtual void exitUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext *ctx) = 0;

  virtual void enterScopedMethodCall(EscriptParser::ScopedMethodCallContext *ctx) = 0;
  virtual void exitScopedMethodCall(EscriptParser::ScopedMethodCallContext *ctx) = 0;

  virtual void enterUnambiguousExpression(EscriptParser::UnambiguousExpressionContext *ctx) = 0;
  virtual void exitUnambiguousExpression(EscriptParser::UnambiguousExpressionContext *ctx) = 0;

  virtual void enterAssignmentOperator(EscriptParser::AssignmentOperatorContext *ctx) = 0;
  virtual void exitAssignmentOperator(EscriptParser::AssignmentOperatorContext *ctx) = 0;

  virtual void enterMembership(EscriptParser::MembershipContext *ctx) = 0;
  virtual void exitMembership(EscriptParser::MembershipContext *ctx) = 0;

  virtual void enterMembershipOperator(EscriptParser::MembershipOperatorContext *ctx) = 0;
  virtual void exitMembershipOperator(EscriptParser::MembershipOperatorContext *ctx) = 0;

  virtual void enterDisjunction(EscriptParser::DisjunctionContext *ctx) = 0;
  virtual void exitDisjunction(EscriptParser::DisjunctionContext *ctx) = 0;

  virtual void enterDisjunctionOperator(EscriptParser::DisjunctionOperatorContext *ctx) = 0;
  virtual void exitDisjunctionOperator(EscriptParser::DisjunctionOperatorContext *ctx) = 0;

  virtual void enterConjunction(EscriptParser::ConjunctionContext *ctx) = 0;
  virtual void exitConjunction(EscriptParser::ConjunctionContext *ctx) = 0;

  virtual void enterConjunctionOperator(EscriptParser::ConjunctionOperatorContext *ctx) = 0;
  virtual void exitConjunctionOperator(EscriptParser::ConjunctionOperatorContext *ctx) = 0;

  virtual void enterEquality(EscriptParser::EqualityContext *ctx) = 0;
  virtual void exitEquality(EscriptParser::EqualityContext *ctx) = 0;

  virtual void enterEqualityOperator(EscriptParser::EqualityOperatorContext *ctx) = 0;
  virtual void exitEqualityOperator(EscriptParser::EqualityOperatorContext *ctx) = 0;

  virtual void enterBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext *ctx) = 0;
  virtual void exitBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext *ctx) = 0;

  virtual void enterBitwiseXor(EscriptParser::BitwiseXorContext *ctx) = 0;
  virtual void exitBitwiseXor(EscriptParser::BitwiseXorContext *ctx) = 0;

  virtual void enterBitwiseConjunction(EscriptParser::BitwiseConjunctionContext *ctx) = 0;
  virtual void exitBitwiseConjunction(EscriptParser::BitwiseConjunctionContext *ctx) = 0;

  virtual void enterBitshiftRight(EscriptParser::BitshiftRightContext *ctx) = 0;
  virtual void exitBitshiftRight(EscriptParser::BitshiftRightContext *ctx) = 0;

  virtual void enterBitshiftLeft(EscriptParser::BitshiftLeftContext *ctx) = 0;
  virtual void exitBitshiftLeft(EscriptParser::BitshiftLeftContext *ctx) = 0;

  virtual void enterComparison(EscriptParser::ComparisonContext *ctx) = 0;
  virtual void exitComparison(EscriptParser::ComparisonContext *ctx) = 0;

  virtual void enterComparisonOperator(EscriptParser::ComparisonOperatorContext *ctx) = 0;
  virtual void exitComparisonOperator(EscriptParser::ComparisonOperatorContext *ctx) = 0;

  virtual void enterInfixOperation(EscriptParser::InfixOperationContext *ctx) = 0;
  virtual void exitInfixOperation(EscriptParser::InfixOperationContext *ctx) = 0;

  virtual void enterInfixOperator(EscriptParser::InfixOperatorContext *ctx) = 0;
  virtual void exitInfixOperator(EscriptParser::InfixOperatorContext *ctx) = 0;

  virtual void enterElvisExpression(EscriptParser::ElvisExpressionContext *ctx) = 0;
  virtual void exitElvisExpression(EscriptParser::ElvisExpressionContext *ctx) = 0;

  virtual void enterAdditiveExpression(EscriptParser::AdditiveExpressionContext *ctx) = 0;
  virtual void exitAdditiveExpression(EscriptParser::AdditiveExpressionContext *ctx) = 0;

  virtual void enterAdditiveOperator(EscriptParser::AdditiveOperatorContext *ctx) = 0;
  virtual void exitAdditiveOperator(EscriptParser::AdditiveOperatorContext *ctx) = 0;

  virtual void enterMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext *ctx) = 0;
  virtual void exitMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext *ctx) = 0;

  virtual void enterMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext *ctx) = 0;
  virtual void exitMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext *ctx) = 0;

  virtual void enterPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext *ctx) = 0;
  virtual void exitPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext *ctx) = 0;

  virtual void enterPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext *ctx) = 0;
  virtual void exitPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext *ctx) = 0;

  virtual void enterPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext *ctx) = 0;
  virtual void exitPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext *ctx) = 0;

  virtual void enterPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext *ctx) = 0;
  virtual void exitPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext *ctx) = 0;

  virtual void enterPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext *ctx) = 0;
  virtual void exitPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext *ctx) = 0;

  virtual void enterPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext *ctx) = 0;
  virtual void exitPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext *ctx) = 0;

  virtual void enterIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) = 0;
  virtual void exitIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) = 0;

  virtual void enterNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) = 0;
  virtual void exitNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) = 0;

  virtual void enterMembershipSuffix(EscriptParser::MembershipSuffixContext *ctx) = 0;
  virtual void exitMembershipSuffix(EscriptParser::MembershipSuffixContext *ctx) = 0;

  virtual void enterMemberCallSuffix(EscriptParser::MemberCallSuffixContext *ctx) = 0;
  virtual void exitMemberCallSuffix(EscriptParser::MemberCallSuffixContext *ctx) = 0;

  virtual void enterMemberAccessOperator(EscriptParser::MemberAccessOperatorContext *ctx) = 0;
  virtual void exitMemberAccessOperator(EscriptParser::MemberAccessOperatorContext *ctx) = 0;

  virtual void enterCallSuffix(EscriptParser::CallSuffixContext *ctx) = 0;
  virtual void exitCallSuffix(EscriptParser::CallSuffixContext *ctx) = 0;

  virtual void enterAtomicExpression(EscriptParser::AtomicExpressionContext *ctx) = 0;
  virtual void exitAtomicExpression(EscriptParser::AtomicExpressionContext *ctx) = 0;

  virtual void enterFunctionReference(EscriptParser::FunctionReferenceContext *ctx) = 0;
  virtual void exitFunctionReference(EscriptParser::FunctionReferenceContext *ctx) = 0;

  virtual void enterUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext *ctx) = 0;
  virtual void exitUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext *ctx) = 0;

  virtual void enterUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext *ctx) = 0;
  virtual void exitUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext *ctx) = 0;

  virtual void enterUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext *ctx) = 0;
  virtual void exitUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext *ctx) = 0;

  virtual void enterUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext *ctx) = 0;
  virtual void exitUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext *ctx) = 0;

  virtual void enterUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext *ctx) = 0;
  virtual void exitUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext *ctx) = 0;

  virtual void enterParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext *ctx) = 0;
  virtual void exitParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext *ctx) = 0;

  virtual void enterExpression(EscriptParser::ExpressionContext *ctx) = 0;
  virtual void exitExpression(EscriptParser::ExpressionContext *ctx) = 0;

  virtual void enterPrimary(EscriptParser::PrimaryContext *ctx) = 0;
  virtual void exitPrimary(EscriptParser::PrimaryContext *ctx) = 0;

  virtual void enterParExpression(EscriptParser::ParExpressionContext *ctx) = 0;
  virtual void exitParExpression(EscriptParser::ParExpressionContext *ctx) = 0;

  virtual void enterExpressionList(EscriptParser::ExpressionListContext *ctx) = 0;
  virtual void exitExpressionList(EscriptParser::ExpressionListContext *ctx) = 0;

  virtual void enterUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext *ctx) = 0;
  virtual void exitUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext *ctx) = 0;

  virtual void enterMethodCallArgument(EscriptParser::MethodCallArgumentContext *ctx) = 0;
  virtual void exitMethodCallArgument(EscriptParser::MethodCallArgumentContext *ctx) = 0;

  virtual void enterMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext *ctx) = 0;
  virtual void exitMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext *ctx) = 0;

  virtual void enterMethodCall(EscriptParser::MethodCallContext *ctx) = 0;
  virtual void exitMethodCall(EscriptParser::MethodCallContext *ctx) = 0;

  virtual void enterValueArgumentList(EscriptParser::ValueArgumentListContext *ctx) = 0;
  virtual void exitValueArgumentList(EscriptParser::ValueArgumentListContext *ctx) = 0;

  virtual void enterValueArguments(EscriptParser::ValueArgumentsContext *ctx) = 0;
  virtual void exitValueArguments(EscriptParser::ValueArgumentsContext *ctx) = 0;

  virtual void enterUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext *ctx) = 0;
  virtual void exitUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext *ctx) = 0;

  virtual void enterUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext *ctx) = 0;
  virtual void exitUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext *ctx) = 0;

  virtual void enterUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext *ctx) = 0;
  virtual void exitUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext *ctx) = 0;

  virtual void enterUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext *ctx) = 0;
  virtual void exitUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext *ctx) = 0;

  virtual void enterMemberCall(EscriptParser::MemberCallContext *ctx) = 0;
  virtual void exitMemberCall(EscriptParser::MemberCallContext *ctx) = 0;

  virtual void enterStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) = 0;
  virtual void exitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) = 0;

  virtual void enterStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) = 0;
  virtual void exitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) = 0;

  virtual void enterStructInitializer(EscriptParser::StructInitializerContext *ctx) = 0;
  virtual void exitStructInitializer(EscriptParser::StructInitializerContext *ctx) = 0;

  virtual void enterUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext *ctx) = 0;
  virtual void exitUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext *ctx) = 0;

  virtual void enterUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext *ctx) = 0;
  virtual void exitUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext *ctx) = 0;

  virtual void enterUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext *ctx) = 0;
  virtual void exitUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext *ctx) = 0;

  virtual void enterDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) = 0;
  virtual void exitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) = 0;

  virtual void enterUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext *ctx) = 0;
  virtual void exitUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext *ctx) = 0;

  virtual void enterDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) = 0;
  virtual void exitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) = 0;

  virtual void enterUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext *ctx) = 0;
  virtual void exitUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext *ctx) = 0;

  virtual void enterDictInitializer(EscriptParser::DictInitializerContext *ctx) = 0;
  virtual void exitDictInitializer(EscriptParser::DictInitializerContext *ctx) = 0;

  virtual void enterUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext *ctx) = 0;
  virtual void exitUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext *ctx) = 0;

  virtual void enterArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) = 0;
  virtual void exitArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) = 0;

  virtual void enterUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext *ctx) = 0;
  virtual void exitUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext *ctx) = 0;

  virtual void enterLiteral(EscriptParser::LiteralContext *ctx) = 0;
  virtual void exitLiteral(EscriptParser::LiteralContext *ctx) = 0;

  virtual void enterIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) = 0;
  virtual void exitIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) = 0;

  virtual void enterFloatLiteral(EscriptParser::FloatLiteralContext *ctx) = 0;
  virtual void exitFloatLiteral(EscriptParser::FloatLiteralContext *ctx) = 0;


};

}  // namespace EscriptGrammar
