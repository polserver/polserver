


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "EscriptParserListener.h"


namespace EscriptGrammar {

/**
 * This class provides an empty implementation of EscriptParserListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  EscriptParserBaseListener : public EscriptParserListener {
public:

  virtual void enterUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext * /*ctx*/) override { }
  virtual void exitUnambiguousCompilationUnit(EscriptParser::UnambiguousCompilationUnitContext * /*ctx*/) override { }

  virtual void enterCompilationUnit(EscriptParser::CompilationUnitContext * /*ctx*/) override { }
  virtual void exitCompilationUnit(EscriptParser::CompilationUnitContext * /*ctx*/) override { }

  virtual void enterModuleUnit(EscriptParser::ModuleUnitContext * /*ctx*/) override { }
  virtual void exitModuleUnit(EscriptParser::ModuleUnitContext * /*ctx*/) override { }

  virtual void enterUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext * /*ctx*/) override { }
  virtual void exitUnambiguousModuleUnit(EscriptParser::UnambiguousModuleUnitContext * /*ctx*/) override { }

  virtual void enterModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext * /*ctx*/) override { }
  virtual void exitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousModuleDeclarationStatement(EscriptParser::UnambiguousModuleDeclarationStatementContext * /*ctx*/) override { }

  virtual void enterModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitUnambiguousModuleFunctionDeclaration(EscriptParser::UnambiguousModuleFunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext * /*ctx*/) override { }
  virtual void exitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext * /*ctx*/) override { }

  virtual void enterUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext * /*ctx*/) override { }
  virtual void exitUnambiguousModuleFunctionParameterList(EscriptParser::UnambiguousModuleFunctionParameterListContext * /*ctx*/) override { }

  virtual void enterModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext * /*ctx*/) override { }
  virtual void exitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext * /*ctx*/) override { }

  virtual void enterUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext * /*ctx*/) override { }
  virtual void exitUnambiguousModuleFunctionParameter(EscriptParser::UnambiguousModuleFunctionParameterContext * /*ctx*/) override { }

  virtual void enterTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext * /*ctx*/) override { }
  virtual void exitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext * /*ctx*/) override { }
  virtual void exitUnambiguousTopLevelDeclaration(EscriptParser::UnambiguousTopLevelDeclarationContext * /*ctx*/) override { }

  virtual void enterFunctionDeclaration(EscriptParser::FunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitFunctionDeclaration(EscriptParser::FunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionDeclaration(EscriptParser::UnambiguousFunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterStringIdentifier(EscriptParser::StringIdentifierContext * /*ctx*/) override { }
  virtual void exitStringIdentifier(EscriptParser::StringIdentifierContext * /*ctx*/) override { }

  virtual void enterUseDeclaration(EscriptParser::UseDeclarationContext * /*ctx*/) override { }
  virtual void exitUseDeclaration(EscriptParser::UseDeclarationContext * /*ctx*/) override { }

  virtual void enterIncludeDeclaration(EscriptParser::IncludeDeclarationContext * /*ctx*/) override { }
  virtual void exitIncludeDeclaration(EscriptParser::IncludeDeclarationContext * /*ctx*/) override { }

  virtual void enterProgramDeclaration(EscriptParser::ProgramDeclarationContext * /*ctx*/) override { }
  virtual void exitProgramDeclaration(EscriptParser::ProgramDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext * /*ctx*/) override { }
  virtual void exitUnambiguousProgramDeclaration(EscriptParser::UnambiguousProgramDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousStatement(EscriptParser::UnambiguousStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousStatement(EscriptParser::UnambiguousStatementContext * /*ctx*/) override { }

  virtual void enterStatement(EscriptParser::StatementContext * /*ctx*/) override { }
  virtual void exitStatement(EscriptParser::StatementContext * /*ctx*/) override { }

  virtual void enterStatementLabel(EscriptParser::StatementLabelContext * /*ctx*/) override { }
  virtual void exitStatementLabel(EscriptParser::StatementLabelContext * /*ctx*/) override { }

  virtual void enterIfStatement(EscriptParser::IfStatementContext * /*ctx*/) override { }
  virtual void exitIfStatement(EscriptParser::IfStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousIfStatement(EscriptParser::UnambiguousIfStatementContext * /*ctx*/) override { }

  virtual void enterGotoStatement(EscriptParser::GotoStatementContext * /*ctx*/) override { }
  virtual void exitGotoStatement(EscriptParser::GotoStatementContext * /*ctx*/) override { }

  virtual void enterReturnStatement(EscriptParser::ReturnStatementContext * /*ctx*/) override { }
  virtual void exitReturnStatement(EscriptParser::ReturnStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousReturnStatement(EscriptParser::UnambiguousReturnStatementContext * /*ctx*/) override { }

  virtual void enterConstStatement(EscriptParser::ConstStatementContext * /*ctx*/) override { }
  virtual void exitConstStatement(EscriptParser::ConstStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousConstStatement(EscriptParser::UnambiguousConstStatementContext * /*ctx*/) override { }

  virtual void enterVarStatement(EscriptParser::VarStatementContext * /*ctx*/) override { }
  virtual void exitVarStatement(EscriptParser::VarStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousVarStatement(EscriptParser::UnambiguousVarStatementContext * /*ctx*/) override { }

  virtual void enterDoStatement(EscriptParser::DoStatementContext * /*ctx*/) override { }
  virtual void exitDoStatement(EscriptParser::DoStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousDoStatement(EscriptParser::UnambiguousDoStatementContext * /*ctx*/) override { }

  virtual void enterWhileStatement(EscriptParser::WhileStatementContext * /*ctx*/) override { }
  virtual void exitWhileStatement(EscriptParser::WhileStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousWhileStatement(EscriptParser::UnambiguousWhileStatementContext * /*ctx*/) override { }

  virtual void enterExitStatement(EscriptParser::ExitStatementContext * /*ctx*/) override { }
  virtual void exitExitStatement(EscriptParser::ExitStatementContext * /*ctx*/) override { }

  virtual void enterDeclareStatement(EscriptParser::DeclareStatementContext * /*ctx*/) override { }
  virtual void exitDeclareStatement(EscriptParser::DeclareStatementContext * /*ctx*/) override { }

  virtual void enterBreakStatement(EscriptParser::BreakStatementContext * /*ctx*/) override { }
  virtual void exitBreakStatement(EscriptParser::BreakStatementContext * /*ctx*/) override { }

  virtual void enterContinueStatement(EscriptParser::ContinueStatementContext * /*ctx*/) override { }
  virtual void exitContinueStatement(EscriptParser::ContinueStatementContext * /*ctx*/) override { }

  virtual void enterForStatement(EscriptParser::ForStatementContext * /*ctx*/) override { }
  virtual void exitForStatement(EscriptParser::ForStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousForStatement(EscriptParser::UnambiguousForStatementContext * /*ctx*/) override { }

  virtual void enterForeachStatement(EscriptParser::ForeachStatementContext * /*ctx*/) override { }
  virtual void exitForeachStatement(EscriptParser::ForeachStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousForeachStatement(EscriptParser::UnambiguousForeachStatementContext * /*ctx*/) override { }

  virtual void enterRepeatStatement(EscriptParser::RepeatStatementContext * /*ctx*/) override { }
  virtual void exitRepeatStatement(EscriptParser::RepeatStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousRepeatStatement(EscriptParser::UnambiguousRepeatStatementContext * /*ctx*/) override { }

  virtual void enterCaseStatement(EscriptParser::CaseStatementContext * /*ctx*/) override { }
  virtual void exitCaseStatement(EscriptParser::CaseStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousCaseStatement(EscriptParser::UnambiguousCaseStatementContext * /*ctx*/) override { }

  virtual void enterEnumStatement(EscriptParser::EnumStatementContext * /*ctx*/) override { }
  virtual void exitEnumStatement(EscriptParser::EnumStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousEnumStatement(EscriptParser::UnambiguousEnumStatementContext * /*ctx*/) override { }

  virtual void enterBlock(EscriptParser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(EscriptParser::BlockContext * /*ctx*/) override { }

  virtual void enterUnambiguousBlock(EscriptParser::UnambiguousBlockContext * /*ctx*/) override { }
  virtual void exitUnambiguousBlock(EscriptParser::UnambiguousBlockContext * /*ctx*/) override { }

  virtual void enterVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext * /*ctx*/) override { }
  virtual void exitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousVariableDeclarationInitializer(EscriptParser::UnambiguousVariableDeclarationInitializerContext * /*ctx*/) override { }

  virtual void enterEnumList(EscriptParser::EnumListContext * /*ctx*/) override { }
  virtual void exitEnumList(EscriptParser::EnumListContext * /*ctx*/) override { }

  virtual void enterUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext * /*ctx*/) override { }
  virtual void exitUnambiguousEnumList(EscriptParser::UnambiguousEnumListContext * /*ctx*/) override { }

  virtual void enterEnumListEntry(EscriptParser::EnumListEntryContext * /*ctx*/) override { }
  virtual void exitEnumListEntry(EscriptParser::EnumListEntryContext * /*ctx*/) override { }

  virtual void enterUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext * /*ctx*/) override { }
  virtual void exitUnambiguousEnumListEntry(EscriptParser::UnambiguousEnumListEntryContext * /*ctx*/) override { }

  virtual void enterSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext * /*ctx*/) override { }
  virtual void exitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext * /*ctx*/) override { }

  virtual void enterUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext * /*ctx*/) override { }
  virtual void exitUnambiguousSwitchBlockStatementGroup(EscriptParser::UnambiguousSwitchBlockStatementGroupContext * /*ctx*/) override { }

  virtual void enterSwitchLabel(EscriptParser::SwitchLabelContext * /*ctx*/) override { }
  virtual void exitSwitchLabel(EscriptParser::SwitchLabelContext * /*ctx*/) override { }

  virtual void enterForGroup(EscriptParser::ForGroupContext * /*ctx*/) override { }
  virtual void exitForGroup(EscriptParser::ForGroupContext * /*ctx*/) override { }

  virtual void enterUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext * /*ctx*/) override { }
  virtual void exitUnambiguousForGroup(EscriptParser::UnambiguousForGroupContext * /*ctx*/) override { }

  virtual void enterBasicForStatement(EscriptParser::BasicForStatementContext * /*ctx*/) override { }
  virtual void exitBasicForStatement(EscriptParser::BasicForStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousBasicForStatement(EscriptParser::UnambiguousBasicForStatementContext * /*ctx*/) override { }

  virtual void enterCstyleForStatement(EscriptParser::CstyleForStatementContext * /*ctx*/) override { }
  virtual void exitCstyleForStatement(EscriptParser::CstyleForStatementContext * /*ctx*/) override { }

  virtual void enterUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext * /*ctx*/) override { }
  virtual void exitUnambiguousCstyleForStatement(EscriptParser::UnambiguousCstyleForStatementContext * /*ctx*/) override { }

  virtual void enterIdentifierList(EscriptParser::IdentifierListContext * /*ctx*/) override { }
  virtual void exitIdentifierList(EscriptParser::IdentifierListContext * /*ctx*/) override { }

  virtual void enterVariableDeclarationList(EscriptParser::VariableDeclarationListContext * /*ctx*/) override { }
  virtual void exitVariableDeclarationList(EscriptParser::VariableDeclarationListContext * /*ctx*/) override { }

  virtual void enterUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext * /*ctx*/) override { }
  virtual void exitUnambiguousVariableDeclarationList(EscriptParser::UnambiguousVariableDeclarationListContext * /*ctx*/) override { }

  virtual void enterVariableDeclaration(EscriptParser::VariableDeclarationContext * /*ctx*/) override { }
  virtual void exitVariableDeclaration(EscriptParser::VariableDeclarationContext * /*ctx*/) override { }

  virtual void enterUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext * /*ctx*/) override { }
  virtual void exitUnambiguousVariableDeclaration(EscriptParser::UnambiguousVariableDeclarationContext * /*ctx*/) override { }

  virtual void enterProgramParameters(EscriptParser::ProgramParametersContext * /*ctx*/) override { }
  virtual void exitProgramParameters(EscriptParser::ProgramParametersContext * /*ctx*/) override { }

  virtual void enterProgramParameterList(EscriptParser::ProgramParameterListContext * /*ctx*/) override { }
  virtual void exitProgramParameterList(EscriptParser::ProgramParameterListContext * /*ctx*/) override { }

  virtual void enterProgramParameter(EscriptParser::ProgramParameterContext * /*ctx*/) override { }
  virtual void exitProgramParameter(EscriptParser::ProgramParameterContext * /*ctx*/) override { }

  virtual void enterUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext * /*ctx*/) override { }
  virtual void exitUnambiguousProgramParameters(EscriptParser::UnambiguousProgramParametersContext * /*ctx*/) override { }

  virtual void enterUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext * /*ctx*/) override { }
  virtual void exitUnambiguousProgramParameterList(EscriptParser::UnambiguousProgramParameterListContext * /*ctx*/) override { }

  virtual void enterUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext * /*ctx*/) override { }
  virtual void exitUnambiguousProgramParameter(EscriptParser::UnambiguousProgramParameterContext * /*ctx*/) override { }

  virtual void enterFunctionParameters(EscriptParser::FunctionParametersContext * /*ctx*/) override { }
  virtual void exitFunctionParameters(EscriptParser::FunctionParametersContext * /*ctx*/) override { }

  virtual void enterFunctionParameterList(EscriptParser::FunctionParameterListContext * /*ctx*/) override { }
  virtual void exitFunctionParameterList(EscriptParser::FunctionParameterListContext * /*ctx*/) override { }

  virtual void enterFunctionParameter(EscriptParser::FunctionParameterContext * /*ctx*/) override { }
  virtual void exitFunctionParameter(EscriptParser::FunctionParameterContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionParameters(EscriptParser::UnambiguousFunctionParametersContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionParameterList(EscriptParser::UnambiguousFunctionParameterListContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionParameter(EscriptParser::UnambiguousFunctionParameterContext * /*ctx*/) override { }

  virtual void enterScopedMethodCall(EscriptParser::ScopedMethodCallContext * /*ctx*/) override { }
  virtual void exitScopedMethodCall(EscriptParser::ScopedMethodCallContext * /*ctx*/) override { }

  virtual void enterUnambiguousExpression(EscriptParser::UnambiguousExpressionContext * /*ctx*/) override { }
  virtual void exitUnambiguousExpression(EscriptParser::UnambiguousExpressionContext * /*ctx*/) override { }

  virtual void enterAssignmentOperator(EscriptParser::AssignmentOperatorContext * /*ctx*/) override { }
  virtual void exitAssignmentOperator(EscriptParser::AssignmentOperatorContext * /*ctx*/) override { }

  virtual void enterMembership(EscriptParser::MembershipContext * /*ctx*/) override { }
  virtual void exitMembership(EscriptParser::MembershipContext * /*ctx*/) override { }

  virtual void enterMembershipOperator(EscriptParser::MembershipOperatorContext * /*ctx*/) override { }
  virtual void exitMembershipOperator(EscriptParser::MembershipOperatorContext * /*ctx*/) override { }

  virtual void enterDisjunction(EscriptParser::DisjunctionContext * /*ctx*/) override { }
  virtual void exitDisjunction(EscriptParser::DisjunctionContext * /*ctx*/) override { }

  virtual void enterDisjunctionOperator(EscriptParser::DisjunctionOperatorContext * /*ctx*/) override { }
  virtual void exitDisjunctionOperator(EscriptParser::DisjunctionOperatorContext * /*ctx*/) override { }

  virtual void enterConjunction(EscriptParser::ConjunctionContext * /*ctx*/) override { }
  virtual void exitConjunction(EscriptParser::ConjunctionContext * /*ctx*/) override { }

  virtual void enterConjunctionOperator(EscriptParser::ConjunctionOperatorContext * /*ctx*/) override { }
  virtual void exitConjunctionOperator(EscriptParser::ConjunctionOperatorContext * /*ctx*/) override { }

  virtual void enterEquality(EscriptParser::EqualityContext * /*ctx*/) override { }
  virtual void exitEquality(EscriptParser::EqualityContext * /*ctx*/) override { }

  virtual void enterEqualityOperator(EscriptParser::EqualityOperatorContext * /*ctx*/) override { }
  virtual void exitEqualityOperator(EscriptParser::EqualityOperatorContext * /*ctx*/) override { }

  virtual void enterBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext * /*ctx*/) override { }
  virtual void exitBitwiseDisjunction(EscriptParser::BitwiseDisjunctionContext * /*ctx*/) override { }

  virtual void enterBitwiseXor(EscriptParser::BitwiseXorContext * /*ctx*/) override { }
  virtual void exitBitwiseXor(EscriptParser::BitwiseXorContext * /*ctx*/) override { }

  virtual void enterBitwiseConjunction(EscriptParser::BitwiseConjunctionContext * /*ctx*/) override { }
  virtual void exitBitwiseConjunction(EscriptParser::BitwiseConjunctionContext * /*ctx*/) override { }

  virtual void enterBitshiftRight(EscriptParser::BitshiftRightContext * /*ctx*/) override { }
  virtual void exitBitshiftRight(EscriptParser::BitshiftRightContext * /*ctx*/) override { }

  virtual void enterBitshiftLeft(EscriptParser::BitshiftLeftContext * /*ctx*/) override { }
  virtual void exitBitshiftLeft(EscriptParser::BitshiftLeftContext * /*ctx*/) override { }

  virtual void enterComparison(EscriptParser::ComparisonContext * /*ctx*/) override { }
  virtual void exitComparison(EscriptParser::ComparisonContext * /*ctx*/) override { }

  virtual void enterComparisonOperator(EscriptParser::ComparisonOperatorContext * /*ctx*/) override { }
  virtual void exitComparisonOperator(EscriptParser::ComparisonOperatorContext * /*ctx*/) override { }

  virtual void enterInfixOperation(EscriptParser::InfixOperationContext * /*ctx*/) override { }
  virtual void exitInfixOperation(EscriptParser::InfixOperationContext * /*ctx*/) override { }

  virtual void enterInfixOperator(EscriptParser::InfixOperatorContext * /*ctx*/) override { }
  virtual void exitInfixOperator(EscriptParser::InfixOperatorContext * /*ctx*/) override { }

  virtual void enterElvisExpression(EscriptParser::ElvisExpressionContext * /*ctx*/) override { }
  virtual void exitElvisExpression(EscriptParser::ElvisExpressionContext * /*ctx*/) override { }

  virtual void enterAdditiveExpression(EscriptParser::AdditiveExpressionContext * /*ctx*/) override { }
  virtual void exitAdditiveExpression(EscriptParser::AdditiveExpressionContext * /*ctx*/) override { }

  virtual void enterAdditiveOperator(EscriptParser::AdditiveOperatorContext * /*ctx*/) override { }
  virtual void exitAdditiveOperator(EscriptParser::AdditiveOperatorContext * /*ctx*/) override { }

  virtual void enterMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext * /*ctx*/) override { }
  virtual void exitMultiplicativeExpression(EscriptParser::MultiplicativeExpressionContext * /*ctx*/) override { }

  virtual void enterMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext * /*ctx*/) override { }
  virtual void exitMultiplicativeOperator(EscriptParser::MultiplicativeOperatorContext * /*ctx*/) override { }

  virtual void enterPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext * /*ctx*/) override { }
  virtual void exitPrefixUnaryInversionExpression(EscriptParser::PrefixUnaryInversionExpressionContext * /*ctx*/) override { }

  virtual void enterPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext * /*ctx*/) override { }
  virtual void exitPrefixUnaryInversionOperator(EscriptParser::PrefixUnaryInversionOperatorContext * /*ctx*/) override { }

  virtual void enterPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext * /*ctx*/) override { }
  virtual void exitPrefixUnaryArithmeticExpression(EscriptParser::PrefixUnaryArithmeticExpressionContext * /*ctx*/) override { }

  virtual void enterPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext * /*ctx*/) override { }
  virtual void exitPrefixUnaryArithmeticOperator(EscriptParser::PrefixUnaryArithmeticOperatorContext * /*ctx*/) override { }

  virtual void enterPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext * /*ctx*/) override { }
  virtual void exitPostfixUnaryExpression(EscriptParser::PostfixUnaryExpressionContext * /*ctx*/) override { }

  virtual void enterPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext * /*ctx*/) override { }
  virtual void exitPostfixUnaryOperator(EscriptParser::PostfixUnaryOperatorContext * /*ctx*/) override { }

  virtual void enterIndexingSuffix(EscriptParser::IndexingSuffixContext * /*ctx*/) override { }
  virtual void exitIndexingSuffix(EscriptParser::IndexingSuffixContext * /*ctx*/) override { }

  virtual void enterNavigationSuffix(EscriptParser::NavigationSuffixContext * /*ctx*/) override { }
  virtual void exitNavigationSuffix(EscriptParser::NavigationSuffixContext * /*ctx*/) override { }

  virtual void enterMembershipSuffix(EscriptParser::MembershipSuffixContext * /*ctx*/) override { }
  virtual void exitMembershipSuffix(EscriptParser::MembershipSuffixContext * /*ctx*/) override { }

  virtual void enterMemberCallSuffix(EscriptParser::MemberCallSuffixContext * /*ctx*/) override { }
  virtual void exitMemberCallSuffix(EscriptParser::MemberCallSuffixContext * /*ctx*/) override { }

  virtual void enterMemberAccessOperator(EscriptParser::MemberAccessOperatorContext * /*ctx*/) override { }
  virtual void exitMemberAccessOperator(EscriptParser::MemberAccessOperatorContext * /*ctx*/) override { }

  virtual void enterCallSuffix(EscriptParser::CallSuffixContext * /*ctx*/) override { }
  virtual void exitCallSuffix(EscriptParser::CallSuffixContext * /*ctx*/) override { }

  virtual void enterAtomicExpression(EscriptParser::AtomicExpressionContext * /*ctx*/) override { }
  virtual void exitAtomicExpression(EscriptParser::AtomicExpressionContext * /*ctx*/) override { }

  virtual void enterFunctionReference(EscriptParser::FunctionReferenceContext * /*ctx*/) override { }
  virtual void exitFunctionReference(EscriptParser::FunctionReferenceContext * /*ctx*/) override { }

  virtual void enterUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousExplicitArrayInitializer(EscriptParser::UnambiguousExplicitArrayInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousExplicitStructInitializer(EscriptParser::UnambiguousExplicitStructInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousExplicitDictInitializer(EscriptParser::UnambiguousExplicitDictInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousExplicitErrorInitializer(EscriptParser::UnambiguousExplicitErrorInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousBareArrayInitializer(EscriptParser::UnambiguousBareArrayInitializerContext * /*ctx*/) override { }

  virtual void enterParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext * /*ctx*/) override { }
  virtual void exitParenthesizedExpression(EscriptParser::ParenthesizedExpressionContext * /*ctx*/) override { }

  virtual void enterExpression(EscriptParser::ExpressionContext * /*ctx*/) override { }
  virtual void exitExpression(EscriptParser::ExpressionContext * /*ctx*/) override { }

  virtual void enterPrimary(EscriptParser::PrimaryContext * /*ctx*/) override { }
  virtual void exitPrimary(EscriptParser::PrimaryContext * /*ctx*/) override { }

  virtual void enterParExpression(EscriptParser::ParExpressionContext * /*ctx*/) override { }
  virtual void exitParExpression(EscriptParser::ParExpressionContext * /*ctx*/) override { }

  virtual void enterExpressionList(EscriptParser::ExpressionListContext * /*ctx*/) override { }
  virtual void exitExpressionList(EscriptParser::ExpressionListContext * /*ctx*/) override { }

  virtual void enterUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext * /*ctx*/) override { }
  virtual void exitUnambiguousExpressionList(EscriptParser::UnambiguousExpressionListContext * /*ctx*/) override { }

  virtual void enterMethodCallArgument(EscriptParser::MethodCallArgumentContext * /*ctx*/) override { }
  virtual void exitMethodCallArgument(EscriptParser::MethodCallArgumentContext * /*ctx*/) override { }

  virtual void enterMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext * /*ctx*/) override { }
  virtual void exitMethodCallArgumentList(EscriptParser::MethodCallArgumentListContext * /*ctx*/) override { }

  virtual void enterMethodCall(EscriptParser::MethodCallContext * /*ctx*/) override { }
  virtual void exitMethodCall(EscriptParser::MethodCallContext * /*ctx*/) override { }

  virtual void enterValueArgumentList(EscriptParser::ValueArgumentListContext * /*ctx*/) override { }
  virtual void exitValueArgumentList(EscriptParser::ValueArgumentListContext * /*ctx*/) override { }

  virtual void enterValueArguments(EscriptParser::ValueArgumentsContext * /*ctx*/) override { }
  virtual void exitValueArguments(EscriptParser::ValueArgumentsContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionCallArgument(EscriptParser::UnambiguousFunctionCallArgumentContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionCall(EscriptParser::UnambiguousFunctionCallContext * /*ctx*/) override { }

  virtual void enterUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext * /*ctx*/) override { }
  virtual void exitUnambiguousScopedFunctionCall(EscriptParser::UnambiguousScopedFunctionCallContext * /*ctx*/) override { }

  virtual void enterUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext * /*ctx*/) override { }
  virtual void exitUnambiguousFunctionCallArgumentList(EscriptParser::UnambiguousFunctionCallArgumentListContext * /*ctx*/) override { }

  virtual void enterMemberCall(EscriptParser::MemberCallContext * /*ctx*/) override { }
  virtual void exitMemberCall(EscriptParser::MemberCallContext * /*ctx*/) override { }

  virtual void enterStructInitializerExpression(EscriptParser::StructInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterStructInitializer(EscriptParser::StructInitializerContext * /*ctx*/) override { }
  virtual void exitStructInitializer(EscriptParser::StructInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitUnambiguousStructInitializerExpression(EscriptParser::UnambiguousStructInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitUnambiguousStructInitializerExpressionList(EscriptParser::UnambiguousStructInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousStructInitializer(EscriptParser::UnambiguousStructInitializerContext * /*ctx*/) override { }

  virtual void enterDictInitializerExpression(EscriptParser::DictInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitUnambiguousDictInitializerExpression(EscriptParser::UnambiguousDictInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitUnambiguousDictInitializerExpressionList(EscriptParser::UnambiguousDictInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterDictInitializer(EscriptParser::DictInitializerContext * /*ctx*/) override { }
  virtual void exitDictInitializer(EscriptParser::DictInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousDictInitializer(EscriptParser::UnambiguousDictInitializerContext * /*ctx*/) override { }

  virtual void enterArrayInitializer(EscriptParser::ArrayInitializerContext * /*ctx*/) override { }
  virtual void exitArrayInitializer(EscriptParser::ArrayInitializerContext * /*ctx*/) override { }

  virtual void enterUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext * /*ctx*/) override { }
  virtual void exitUnambiguousArrayInitializer(EscriptParser::UnambiguousArrayInitializerContext * /*ctx*/) override { }

  virtual void enterLiteral(EscriptParser::LiteralContext * /*ctx*/) override { }
  virtual void exitLiteral(EscriptParser::LiteralContext * /*ctx*/) override { }

  virtual void enterIntegerLiteral(EscriptParser::IntegerLiteralContext * /*ctx*/) override { }
  virtual void exitIntegerLiteral(EscriptParser::IntegerLiteralContext * /*ctx*/) override { }

  virtual void enterFloatLiteral(EscriptParser::FloatLiteralContext * /*ctx*/) override { }
  virtual void exitFloatLiteral(EscriptParser::FloatLiteralContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

}  // namespace EscriptGrammar
