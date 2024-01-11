


// Generated from EscriptParser.g4 by ANTLR 4.13.1

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

  virtual void enterCompilationUnit(EscriptParser::CompilationUnitContext * /*ctx*/) override { }
  virtual void exitCompilationUnit(EscriptParser::CompilationUnitContext * /*ctx*/) override { }

  virtual void enterModuleUnit(EscriptParser::ModuleUnitContext * /*ctx*/) override { }
  virtual void exitModuleUnit(EscriptParser::ModuleUnitContext * /*ctx*/) override { }

  virtual void enterModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext * /*ctx*/) override { }
  virtual void exitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext * /*ctx*/) override { }

  virtual void enterModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext * /*ctx*/) override { }
  virtual void exitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext * /*ctx*/) override { }

  virtual void enterModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext * /*ctx*/) override { }
  virtual void exitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext * /*ctx*/) override { }

  virtual void enterTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext * /*ctx*/) override { }
  virtual void exitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext * /*ctx*/) override { }

  virtual void enterFunctionDeclaration(EscriptParser::FunctionDeclarationContext * /*ctx*/) override { }
  virtual void exitFunctionDeclaration(EscriptParser::FunctionDeclarationContext * /*ctx*/) override { }

  virtual void enterStringIdentifier(EscriptParser::StringIdentifierContext * /*ctx*/) override { }
  virtual void exitStringIdentifier(EscriptParser::StringIdentifierContext * /*ctx*/) override { }

  virtual void enterUseDeclaration(EscriptParser::UseDeclarationContext * /*ctx*/) override { }
  virtual void exitUseDeclaration(EscriptParser::UseDeclarationContext * /*ctx*/) override { }

  virtual void enterIncludeDeclaration(EscriptParser::IncludeDeclarationContext * /*ctx*/) override { }
  virtual void exitIncludeDeclaration(EscriptParser::IncludeDeclarationContext * /*ctx*/) override { }

  virtual void enterProgramDeclaration(EscriptParser::ProgramDeclarationContext * /*ctx*/) override { }
  virtual void exitProgramDeclaration(EscriptParser::ProgramDeclarationContext * /*ctx*/) override { }

  virtual void enterStatement(EscriptParser::StatementContext * /*ctx*/) override { }
  virtual void exitStatement(EscriptParser::StatementContext * /*ctx*/) override { }

  virtual void enterStatementLabel(EscriptParser::StatementLabelContext * /*ctx*/) override { }
  virtual void exitStatementLabel(EscriptParser::StatementLabelContext * /*ctx*/) override { }

  virtual void enterIfStatement(EscriptParser::IfStatementContext * /*ctx*/) override { }
  virtual void exitIfStatement(EscriptParser::IfStatementContext * /*ctx*/) override { }

  virtual void enterGotoStatement(EscriptParser::GotoStatementContext * /*ctx*/) override { }
  virtual void exitGotoStatement(EscriptParser::GotoStatementContext * /*ctx*/) override { }

  virtual void enterReturnStatement(EscriptParser::ReturnStatementContext * /*ctx*/) override { }
  virtual void exitReturnStatement(EscriptParser::ReturnStatementContext * /*ctx*/) override { }

  virtual void enterConstStatement(EscriptParser::ConstStatementContext * /*ctx*/) override { }
  virtual void exitConstStatement(EscriptParser::ConstStatementContext * /*ctx*/) override { }

  virtual void enterVarStatement(EscriptParser::VarStatementContext * /*ctx*/) override { }
  virtual void exitVarStatement(EscriptParser::VarStatementContext * /*ctx*/) override { }

  virtual void enterDoStatement(EscriptParser::DoStatementContext * /*ctx*/) override { }
  virtual void exitDoStatement(EscriptParser::DoStatementContext * /*ctx*/) override { }

  virtual void enterWhileStatement(EscriptParser::WhileStatementContext * /*ctx*/) override { }
  virtual void exitWhileStatement(EscriptParser::WhileStatementContext * /*ctx*/) override { }

  virtual void enterExitStatement(EscriptParser::ExitStatementContext * /*ctx*/) override { }
  virtual void exitExitStatement(EscriptParser::ExitStatementContext * /*ctx*/) override { }

  virtual void enterBreakStatement(EscriptParser::BreakStatementContext * /*ctx*/) override { }
  virtual void exitBreakStatement(EscriptParser::BreakStatementContext * /*ctx*/) override { }

  virtual void enterContinueStatement(EscriptParser::ContinueStatementContext * /*ctx*/) override { }
  virtual void exitContinueStatement(EscriptParser::ContinueStatementContext * /*ctx*/) override { }

  virtual void enterForStatement(EscriptParser::ForStatementContext * /*ctx*/) override { }
  virtual void exitForStatement(EscriptParser::ForStatementContext * /*ctx*/) override { }

  virtual void enterForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext * /*ctx*/) override { }
  virtual void exitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext * /*ctx*/) override { }

  virtual void enterForeachStatement(EscriptParser::ForeachStatementContext * /*ctx*/) override { }
  virtual void exitForeachStatement(EscriptParser::ForeachStatementContext * /*ctx*/) override { }

  virtual void enterRepeatStatement(EscriptParser::RepeatStatementContext * /*ctx*/) override { }
  virtual void exitRepeatStatement(EscriptParser::RepeatStatementContext * /*ctx*/) override { }

  virtual void enterCaseStatement(EscriptParser::CaseStatementContext * /*ctx*/) override { }
  virtual void exitCaseStatement(EscriptParser::CaseStatementContext * /*ctx*/) override { }

  virtual void enterEnumStatement(EscriptParser::EnumStatementContext * /*ctx*/) override { }
  virtual void exitEnumStatement(EscriptParser::EnumStatementContext * /*ctx*/) override { }

  virtual void enterBlock(EscriptParser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(EscriptParser::BlockContext * /*ctx*/) override { }

  virtual void enterVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext * /*ctx*/) override { }
  virtual void exitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext * /*ctx*/) override { }

  virtual void enterEnumList(EscriptParser::EnumListContext * /*ctx*/) override { }
  virtual void exitEnumList(EscriptParser::EnumListContext * /*ctx*/) override { }

  virtual void enterEnumListEntry(EscriptParser::EnumListEntryContext * /*ctx*/) override { }
  virtual void exitEnumListEntry(EscriptParser::EnumListEntryContext * /*ctx*/) override { }

  virtual void enterSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext * /*ctx*/) override { }
  virtual void exitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext * /*ctx*/) override { }

  virtual void enterSwitchLabel(EscriptParser::SwitchLabelContext * /*ctx*/) override { }
  virtual void exitSwitchLabel(EscriptParser::SwitchLabelContext * /*ctx*/) override { }

  virtual void enterForGroup(EscriptParser::ForGroupContext * /*ctx*/) override { }
  virtual void exitForGroup(EscriptParser::ForGroupContext * /*ctx*/) override { }

  virtual void enterBasicForStatement(EscriptParser::BasicForStatementContext * /*ctx*/) override { }
  virtual void exitBasicForStatement(EscriptParser::BasicForStatementContext * /*ctx*/) override { }

  virtual void enterCstyleForStatement(EscriptParser::CstyleForStatementContext * /*ctx*/) override { }
  virtual void exitCstyleForStatement(EscriptParser::CstyleForStatementContext * /*ctx*/) override { }

  virtual void enterIdentifierList(EscriptParser::IdentifierListContext * /*ctx*/) override { }
  virtual void exitIdentifierList(EscriptParser::IdentifierListContext * /*ctx*/) override { }

  virtual void enterVariableDeclarationList(EscriptParser::VariableDeclarationListContext * /*ctx*/) override { }
  virtual void exitVariableDeclarationList(EscriptParser::VariableDeclarationListContext * /*ctx*/) override { }

  virtual void enterConstantDeclaration(EscriptParser::ConstantDeclarationContext * /*ctx*/) override { }
  virtual void exitConstantDeclaration(EscriptParser::ConstantDeclarationContext * /*ctx*/) override { }

  virtual void enterVariableDeclaration(EscriptParser::VariableDeclarationContext * /*ctx*/) override { }
  virtual void exitVariableDeclaration(EscriptParser::VariableDeclarationContext * /*ctx*/) override { }

  virtual void enterProgramParameters(EscriptParser::ProgramParametersContext * /*ctx*/) override { }
  virtual void exitProgramParameters(EscriptParser::ProgramParametersContext * /*ctx*/) override { }

  virtual void enterProgramParameterList(EscriptParser::ProgramParameterListContext * /*ctx*/) override { }
  virtual void exitProgramParameterList(EscriptParser::ProgramParameterListContext * /*ctx*/) override { }

  virtual void enterProgramParameter(EscriptParser::ProgramParameterContext * /*ctx*/) override { }
  virtual void exitProgramParameter(EscriptParser::ProgramParameterContext * /*ctx*/) override { }

  virtual void enterFunctionParameters(EscriptParser::FunctionParametersContext * /*ctx*/) override { }
  virtual void exitFunctionParameters(EscriptParser::FunctionParametersContext * /*ctx*/) override { }

  virtual void enterFunctionParameterList(EscriptParser::FunctionParameterListContext * /*ctx*/) override { }
  virtual void exitFunctionParameterList(EscriptParser::FunctionParameterListContext * /*ctx*/) override { }

  virtual void enterFunctionParameter(EscriptParser::FunctionParameterContext * /*ctx*/) override { }
  virtual void exitFunctionParameter(EscriptParser::FunctionParameterContext * /*ctx*/) override { }

  virtual void enterScopedFunctionCall(EscriptParser::ScopedFunctionCallContext * /*ctx*/) override { }
  virtual void exitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext * /*ctx*/) override { }

  virtual void enterFunctionReference(EscriptParser::FunctionReferenceContext * /*ctx*/) override { }
  virtual void exitFunctionReference(EscriptParser::FunctionReferenceContext * /*ctx*/) override { }

  virtual void enterExpression(EscriptParser::ExpressionContext * /*ctx*/) override { }
  virtual void exitExpression(EscriptParser::ExpressionContext * /*ctx*/) override { }

  virtual void enterPrimary(EscriptParser::PrimaryContext * /*ctx*/) override { }
  virtual void exitPrimary(EscriptParser::PrimaryContext * /*ctx*/) override { }

  virtual void enterExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext * /*ctx*/) override { }
  virtual void exitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext * /*ctx*/) override { }

  virtual void enterExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext * /*ctx*/) override { }
  virtual void exitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext * /*ctx*/) override { }

  virtual void enterExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext * /*ctx*/) override { }
  virtual void exitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext * /*ctx*/) override { }

  virtual void enterExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext * /*ctx*/) override { }
  virtual void exitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext * /*ctx*/) override { }

  virtual void enterBareArrayInitializer(EscriptParser::BareArrayInitializerContext * /*ctx*/) override { }
  virtual void exitBareArrayInitializer(EscriptParser::BareArrayInitializerContext * /*ctx*/) override { }

  virtual void enterParExpression(EscriptParser::ParExpressionContext * /*ctx*/) override { }
  virtual void exitParExpression(EscriptParser::ParExpressionContext * /*ctx*/) override { }

  virtual void enterExpressionList(EscriptParser::ExpressionListContext * /*ctx*/) override { }
  virtual void exitExpressionList(EscriptParser::ExpressionListContext * /*ctx*/) override { }

  virtual void enterExpressionSuffix(EscriptParser::ExpressionSuffixContext * /*ctx*/) override { }
  virtual void exitExpressionSuffix(EscriptParser::ExpressionSuffixContext * /*ctx*/) override { }

  virtual void enterIndexingSuffix(EscriptParser::IndexingSuffixContext * /*ctx*/) override { }
  virtual void exitIndexingSuffix(EscriptParser::IndexingSuffixContext * /*ctx*/) override { }

  virtual void enterNavigationSuffix(EscriptParser::NavigationSuffixContext * /*ctx*/) override { }
  virtual void exitNavigationSuffix(EscriptParser::NavigationSuffixContext * /*ctx*/) override { }

  virtual void enterMethodCallSuffix(EscriptParser::MethodCallSuffixContext * /*ctx*/) override { }
  virtual void exitMethodCallSuffix(EscriptParser::MethodCallSuffixContext * /*ctx*/) override { }

  virtual void enterFunctionCall(EscriptParser::FunctionCallContext * /*ctx*/) override { }
  virtual void exitFunctionCall(EscriptParser::FunctionCallContext * /*ctx*/) override { }

  virtual void enterStructInitializerExpression(EscriptParser::StructInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterStructInitializer(EscriptParser::StructInitializerContext * /*ctx*/) override { }
  virtual void exitStructInitializer(EscriptParser::StructInitializerContext * /*ctx*/) override { }

  virtual void enterDictInitializerExpression(EscriptParser::DictInitializerExpressionContext * /*ctx*/) override { }
  virtual void exitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext * /*ctx*/) override { }

  virtual void enterDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext * /*ctx*/) override { }
  virtual void exitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext * /*ctx*/) override { }

  virtual void enterDictInitializer(EscriptParser::DictInitializerContext * /*ctx*/) override { }
  virtual void exitDictInitializer(EscriptParser::DictInitializerContext * /*ctx*/) override { }

  virtual void enterArrayInitializer(EscriptParser::ArrayInitializerContext * /*ctx*/) override { }
  virtual void exitArrayInitializer(EscriptParser::ArrayInitializerContext * /*ctx*/) override { }

  virtual void enterLiteral(EscriptParser::LiteralContext * /*ctx*/) override { }
  virtual void exitLiteral(EscriptParser::LiteralContext * /*ctx*/) override { }

  virtual void enterInterpolatedString(EscriptParser::InterpolatedStringContext * /*ctx*/) override { }
  virtual void exitInterpolatedString(EscriptParser::InterpolatedStringContext * /*ctx*/) override { }

  virtual void enterInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext * /*ctx*/) override { }
  virtual void exitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext * /*ctx*/) override { }

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
