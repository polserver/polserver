


// Generated from EscriptParser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "EscriptParser.h"


namespace EscriptGrammar {

/**
 * This interface defines an abstract listener for a parse tree produced by EscriptParser.
 */
class  EscriptParserListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterCompilationUnit(EscriptParser::CompilationUnitContext *ctx) = 0;
  virtual void exitCompilationUnit(EscriptParser::CompilationUnitContext *ctx) = 0;

  virtual void enterModuleUnit(EscriptParser::ModuleUnitContext *ctx) = 0;
  virtual void exitModuleUnit(EscriptParser::ModuleUnitContext *ctx) = 0;

  virtual void enterEvaluateUnit(EscriptParser::EvaluateUnitContext *ctx) = 0;
  virtual void exitEvaluateUnit(EscriptParser::EvaluateUnitContext *ctx) = 0;

  virtual void enterModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) = 0;
  virtual void exitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *ctx) = 0;

  virtual void enterModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) = 0;
  virtual void exitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *ctx) = 0;

  virtual void enterModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) = 0;
  virtual void exitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *ctx) = 0;

  virtual void enterModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) = 0;
  virtual void exitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *ctx) = 0;

  virtual void enterTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) = 0;
  virtual void exitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *ctx) = 0;

  virtual void enterClassDeclaration(EscriptParser::ClassDeclarationContext *ctx) = 0;
  virtual void exitClassDeclaration(EscriptParser::ClassDeclarationContext *ctx) = 0;

  virtual void enterClassParameters(EscriptParser::ClassParametersContext *ctx) = 0;
  virtual void exitClassParameters(EscriptParser::ClassParametersContext *ctx) = 0;

  virtual void enterClassParameterList(EscriptParser::ClassParameterListContext *ctx) = 0;
  virtual void exitClassParameterList(EscriptParser::ClassParameterListContext *ctx) = 0;

  virtual void enterClassBody(EscriptParser::ClassBodyContext *ctx) = 0;
  virtual void exitClassBody(EscriptParser::ClassBodyContext *ctx) = 0;

  virtual void enterClassStatement(EscriptParser::ClassStatementContext *ctx) = 0;
  virtual void exitClassStatement(EscriptParser::ClassStatementContext *ctx) = 0;

  virtual void enterFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) = 0;
  virtual void exitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *ctx) = 0;

  virtual void enterStringIdentifier(EscriptParser::StringIdentifierContext *ctx) = 0;
  virtual void exitStringIdentifier(EscriptParser::StringIdentifierContext *ctx) = 0;

  virtual void enterUseDeclaration(EscriptParser::UseDeclarationContext *ctx) = 0;
  virtual void exitUseDeclaration(EscriptParser::UseDeclarationContext *ctx) = 0;

  virtual void enterIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) = 0;
  virtual void exitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *ctx) = 0;

  virtual void enterProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) = 0;
  virtual void exitProgramDeclaration(EscriptParser::ProgramDeclarationContext *ctx) = 0;

  virtual void enterStatement(EscriptParser::StatementContext *ctx) = 0;
  virtual void exitStatement(EscriptParser::StatementContext *ctx) = 0;

  virtual void enterStatementLabel(EscriptParser::StatementLabelContext *ctx) = 0;
  virtual void exitStatementLabel(EscriptParser::StatementLabelContext *ctx) = 0;

  virtual void enterIfStatement(EscriptParser::IfStatementContext *ctx) = 0;
  virtual void exitIfStatement(EscriptParser::IfStatementContext *ctx) = 0;

  virtual void enterGotoStatement(EscriptParser::GotoStatementContext *ctx) = 0;
  virtual void exitGotoStatement(EscriptParser::GotoStatementContext *ctx) = 0;

  virtual void enterReturnStatement(EscriptParser::ReturnStatementContext *ctx) = 0;
  virtual void exitReturnStatement(EscriptParser::ReturnStatementContext *ctx) = 0;

  virtual void enterConstStatement(EscriptParser::ConstStatementContext *ctx) = 0;
  virtual void exitConstStatement(EscriptParser::ConstStatementContext *ctx) = 0;

  virtual void enterVarStatement(EscriptParser::VarStatementContext *ctx) = 0;
  virtual void exitVarStatement(EscriptParser::VarStatementContext *ctx) = 0;

  virtual void enterDoStatement(EscriptParser::DoStatementContext *ctx) = 0;
  virtual void exitDoStatement(EscriptParser::DoStatementContext *ctx) = 0;

  virtual void enterWhileStatement(EscriptParser::WhileStatementContext *ctx) = 0;
  virtual void exitWhileStatement(EscriptParser::WhileStatementContext *ctx) = 0;

  virtual void enterExitStatement(EscriptParser::ExitStatementContext *ctx) = 0;
  virtual void exitExitStatement(EscriptParser::ExitStatementContext *ctx) = 0;

  virtual void enterBreakStatement(EscriptParser::BreakStatementContext *ctx) = 0;
  virtual void exitBreakStatement(EscriptParser::BreakStatementContext *ctx) = 0;

  virtual void enterContinueStatement(EscriptParser::ContinueStatementContext *ctx) = 0;
  virtual void exitContinueStatement(EscriptParser::ContinueStatementContext *ctx) = 0;

  virtual void enterForStatement(EscriptParser::ForStatementContext *ctx) = 0;
  virtual void exitForStatement(EscriptParser::ForStatementContext *ctx) = 0;

  virtual void enterForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *ctx) = 0;
  virtual void exitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *ctx) = 0;

  virtual void enterForeachStatement(EscriptParser::ForeachStatementContext *ctx) = 0;
  virtual void exitForeachStatement(EscriptParser::ForeachStatementContext *ctx) = 0;

  virtual void enterRepeatStatement(EscriptParser::RepeatStatementContext *ctx) = 0;
  virtual void exitRepeatStatement(EscriptParser::RepeatStatementContext *ctx) = 0;

  virtual void enterCaseStatement(EscriptParser::CaseStatementContext *ctx) = 0;
  virtual void exitCaseStatement(EscriptParser::CaseStatementContext *ctx) = 0;

  virtual void enterEnumStatement(EscriptParser::EnumStatementContext *ctx) = 0;
  virtual void exitEnumStatement(EscriptParser::EnumStatementContext *ctx) = 0;

  virtual void enterBlock(EscriptParser::BlockContext *ctx) = 0;
  virtual void exitBlock(EscriptParser::BlockContext *ctx) = 0;

  virtual void enterVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) = 0;
  virtual void exitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *ctx) = 0;

  virtual void enterEnumList(EscriptParser::EnumListContext *ctx) = 0;
  virtual void exitEnumList(EscriptParser::EnumListContext *ctx) = 0;

  virtual void enterEnumListEntry(EscriptParser::EnumListEntryContext *ctx) = 0;
  virtual void exitEnumListEntry(EscriptParser::EnumListEntryContext *ctx) = 0;

  virtual void enterSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) = 0;
  virtual void exitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *ctx) = 0;

  virtual void enterSwitchLabel(EscriptParser::SwitchLabelContext *ctx) = 0;
  virtual void exitSwitchLabel(EscriptParser::SwitchLabelContext *ctx) = 0;

  virtual void enterForGroup(EscriptParser::ForGroupContext *ctx) = 0;
  virtual void exitForGroup(EscriptParser::ForGroupContext *ctx) = 0;

  virtual void enterBasicForStatement(EscriptParser::BasicForStatementContext *ctx) = 0;
  virtual void exitBasicForStatement(EscriptParser::BasicForStatementContext *ctx) = 0;

  virtual void enterCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) = 0;
  virtual void exitCstyleForStatement(EscriptParser::CstyleForStatementContext *ctx) = 0;

  virtual void enterIdentifierList(EscriptParser::IdentifierListContext *ctx) = 0;
  virtual void exitIdentifierList(EscriptParser::IdentifierListContext *ctx) = 0;

  virtual void enterVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) = 0;
  virtual void exitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *ctx) = 0;

  virtual void enterConstantDeclaration(EscriptParser::ConstantDeclarationContext *ctx) = 0;
  virtual void exitConstantDeclaration(EscriptParser::ConstantDeclarationContext *ctx) = 0;

  virtual void enterVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) = 0;
  virtual void exitVariableDeclaration(EscriptParser::VariableDeclarationContext *ctx) = 0;

  virtual void enterProgramParameters(EscriptParser::ProgramParametersContext *ctx) = 0;
  virtual void exitProgramParameters(EscriptParser::ProgramParametersContext *ctx) = 0;

  virtual void enterProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) = 0;
  virtual void exitProgramParameterList(EscriptParser::ProgramParameterListContext *ctx) = 0;

  virtual void enterProgramParameter(EscriptParser::ProgramParameterContext *ctx) = 0;
  virtual void exitProgramParameter(EscriptParser::ProgramParameterContext *ctx) = 0;

  virtual void enterFunctionParameters(EscriptParser::FunctionParametersContext *ctx) = 0;
  virtual void exitFunctionParameters(EscriptParser::FunctionParametersContext *ctx) = 0;

  virtual void enterFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) = 0;
  virtual void exitFunctionParameterList(EscriptParser::FunctionParameterListContext *ctx) = 0;

  virtual void enterFunctionParameter(EscriptParser::FunctionParameterContext *ctx) = 0;
  virtual void exitFunctionParameter(EscriptParser::FunctionParameterContext *ctx) = 0;

  virtual void enterScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *ctx) = 0;
  virtual void exitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *ctx) = 0;

  virtual void enterFunctionReference(EscriptParser::FunctionReferenceContext *ctx) = 0;
  virtual void exitFunctionReference(EscriptParser::FunctionReferenceContext *ctx) = 0;

  virtual void enterExpression(EscriptParser::ExpressionContext *ctx) = 0;
  virtual void exitExpression(EscriptParser::ExpressionContext *ctx) = 0;

  virtual void enterPrimary(EscriptParser::PrimaryContext *ctx) = 0;
  virtual void exitPrimary(EscriptParser::PrimaryContext *ctx) = 0;

  virtual void enterScopedIdentifier(EscriptParser::ScopedIdentifierContext *ctx) = 0;
  virtual void exitScopedIdentifier(EscriptParser::ScopedIdentifierContext *ctx) = 0;

  virtual void enterFunctionExpression(EscriptParser::FunctionExpressionContext *ctx) = 0;
  virtual void exitFunctionExpression(EscriptParser::FunctionExpressionContext *ctx) = 0;

  virtual void enterExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *ctx) = 0;
  virtual void exitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *ctx) = 0;

  virtual void enterExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *ctx) = 0;
  virtual void exitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *ctx) = 0;

  virtual void enterExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *ctx) = 0;
  virtual void exitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *ctx) = 0;

  virtual void enterExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *ctx) = 0;
  virtual void exitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *ctx) = 0;

  virtual void enterBareArrayInitializer(EscriptParser::BareArrayInitializerContext *ctx) = 0;
  virtual void exitBareArrayInitializer(EscriptParser::BareArrayInitializerContext *ctx) = 0;

  virtual void enterParExpression(EscriptParser::ParExpressionContext *ctx) = 0;
  virtual void exitParExpression(EscriptParser::ParExpressionContext *ctx) = 0;

  virtual void enterExpressionList(EscriptParser::ExpressionListContext *ctx) = 0;
  virtual void exitExpressionList(EscriptParser::ExpressionListContext *ctx) = 0;

  virtual void enterExpressionListEntry(EscriptParser::ExpressionListEntryContext *ctx) = 0;
  virtual void exitExpressionListEntry(EscriptParser::ExpressionListEntryContext *ctx) = 0;

  virtual void enterExpressionSuffix(EscriptParser::ExpressionSuffixContext *ctx) = 0;
  virtual void exitExpressionSuffix(EscriptParser::ExpressionSuffixContext *ctx) = 0;

  virtual void enterIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) = 0;
  virtual void exitIndexingSuffix(EscriptParser::IndexingSuffixContext *ctx) = 0;

  virtual void enterIndexList(EscriptParser::IndexListContext *ctx) = 0;
  virtual void exitIndexList(EscriptParser::IndexListContext *ctx) = 0;

  virtual void enterNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) = 0;
  virtual void exitNavigationSuffix(EscriptParser::NavigationSuffixContext *ctx) = 0;

  virtual void enterMethodCallSuffix(EscriptParser::MethodCallSuffixContext *ctx) = 0;
  virtual void exitMethodCallSuffix(EscriptParser::MethodCallSuffixContext *ctx) = 0;

  virtual void enterFunctionCallSuffix(EscriptParser::FunctionCallSuffixContext *ctx) = 0;
  virtual void exitFunctionCallSuffix(EscriptParser::FunctionCallSuffixContext *ctx) = 0;

  virtual void enterFunctionCall(EscriptParser::FunctionCallContext *ctx) = 0;
  virtual void exitFunctionCall(EscriptParser::FunctionCallContext *ctx) = 0;

  virtual void enterStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) = 0;
  virtual void exitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *ctx) = 0;

  virtual void enterStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) = 0;
  virtual void exitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *ctx) = 0;

  virtual void enterStructInitializer(EscriptParser::StructInitializerContext *ctx) = 0;
  virtual void exitStructInitializer(EscriptParser::StructInitializerContext *ctx) = 0;

  virtual void enterDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) = 0;
  virtual void exitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *ctx) = 0;

  virtual void enterDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) = 0;
  virtual void exitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *ctx) = 0;

  virtual void enterDictInitializer(EscriptParser::DictInitializerContext *ctx) = 0;
  virtual void exitDictInitializer(EscriptParser::DictInitializerContext *ctx) = 0;

  virtual void enterArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) = 0;
  virtual void exitArrayInitializer(EscriptParser::ArrayInitializerContext *ctx) = 0;

  virtual void enterLiteral(EscriptParser::LiteralContext *ctx) = 0;
  virtual void exitLiteral(EscriptParser::LiteralContext *ctx) = 0;

  virtual void enterInterpolatedString(EscriptParser::InterpolatedStringContext *ctx) = 0;
  virtual void exitInterpolatedString(EscriptParser::InterpolatedStringContext *ctx) = 0;

  virtual void enterInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *ctx) = 0;
  virtual void exitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *ctx) = 0;

  virtual void enterIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) = 0;
  virtual void exitIntegerLiteral(EscriptParser::IntegerLiteralContext *ctx) = 0;

  virtual void enterFloatLiteral(EscriptParser::FloatLiteralContext *ctx) = 0;
  virtual void exitFloatLiteral(EscriptParser::FloatLiteralContext *ctx) = 0;

  virtual void enterBoolLiteral(EscriptParser::BoolLiteralContext *ctx) = 0;
  virtual void exitBoolLiteral(EscriptParser::BoolLiteralContext *ctx) = 0;

  virtual void enterTypeParameters(EscriptParser::TypeParametersContext *ctx) = 0;
  virtual void exitTypeParameters(EscriptParser::TypeParametersContext *ctx) = 0;

  virtual void enterTypeParameterList(EscriptParser::TypeParameterListContext *ctx) = 0;
  virtual void exitTypeParameterList(EscriptParser::TypeParameterListContext *ctx) = 0;

  virtual void enterTypeParameter(EscriptParser::TypeParameterContext *ctx) = 0;
  virtual void exitTypeParameter(EscriptParser::TypeParameterContext *ctx) = 0;

  virtual void enterConstraint(EscriptParser::ConstraintContext *ctx) = 0;
  virtual void exitConstraint(EscriptParser::ConstraintContext *ctx) = 0;

  virtual void enterTypeArguments(EscriptParser::TypeArgumentsContext *ctx) = 0;
  virtual void exitTypeArguments(EscriptParser::TypeArgumentsContext *ctx) = 0;

  virtual void enterTypeArgumentList(EscriptParser::TypeArgumentListContext *ctx) = 0;
  virtual void exitTypeArgumentList(EscriptParser::TypeArgumentListContext *ctx) = 0;

  virtual void enterTypeArgument(EscriptParser::TypeArgumentContext *ctx) = 0;
  virtual void exitTypeArgument(EscriptParser::TypeArgumentContext *ctx) = 0;

  virtual void enterType(EscriptParser::TypeContext *ctx) = 0;
  virtual void exitType(EscriptParser::TypeContext *ctx) = 0;

  virtual void enterBinaryOrPrimaryType(EscriptParser::BinaryOrPrimaryTypeContext *ctx) = 0;
  virtual void exitBinaryOrPrimaryType(EscriptParser::BinaryOrPrimaryTypeContext *ctx) = 0;

  virtual void enterPrimaryType(EscriptParser::PrimaryTypeContext *ctx) = 0;
  virtual void exitPrimaryType(EscriptParser::PrimaryTypeContext *ctx) = 0;

  virtual void enterPredefinedType(EscriptParser::PredefinedTypeContext *ctx) = 0;
  virtual void exitPredefinedType(EscriptParser::PredefinedTypeContext *ctx) = 0;

  virtual void enterTypeReference(EscriptParser::TypeReferenceContext *ctx) = 0;
  virtual void exitTypeReference(EscriptParser::TypeReferenceContext *ctx) = 0;

  virtual void enterTypeGeneric(EscriptParser::TypeGenericContext *ctx) = 0;
  virtual void exitTypeGeneric(EscriptParser::TypeGenericContext *ctx) = 0;

  virtual void enterObjectType(EscriptParser::ObjectTypeContext *ctx) = 0;
  virtual void exitObjectType(EscriptParser::ObjectTypeContext *ctx) = 0;

  virtual void enterTypeBody(EscriptParser::TypeBodyContext *ctx) = 0;
  virtual void exitTypeBody(EscriptParser::TypeBodyContext *ctx) = 0;

  virtual void enterTypeMemberList(EscriptParser::TypeMemberListContext *ctx) = 0;
  virtual void exitTypeMemberList(EscriptParser::TypeMemberListContext *ctx) = 0;

  virtual void enterTypeMember(EscriptParser::TypeMemberContext *ctx) = 0;
  virtual void exitTypeMember(EscriptParser::TypeMemberContext *ctx) = 0;

  virtual void enterTupleElementTypes(EscriptParser::TupleElementTypesContext *ctx) = 0;
  virtual void exitTupleElementTypes(EscriptParser::TupleElementTypesContext *ctx) = 0;

  virtual void enterFunctionType(EscriptParser::FunctionTypeContext *ctx) = 0;
  virtual void exitFunctionType(EscriptParser::FunctionTypeContext *ctx) = 0;

  virtual void enterIdentifierName(EscriptParser::IdentifierNameContext *ctx) = 0;
  virtual void exitIdentifierName(EscriptParser::IdentifierNameContext *ctx) = 0;

  virtual void enterReservedWord(EscriptParser::ReservedWordContext *ctx) = 0;
  virtual void exitReservedWord(EscriptParser::ReservedWordContext *ctx) = 0;

  virtual void enterPropertySignature(EscriptParser::PropertySignatureContext *ctx) = 0;
  virtual void exitPropertySignature(EscriptParser::PropertySignatureContext *ctx) = 0;

  virtual void enterPropertyName(EscriptParser::PropertyNameContext *ctx) = 0;
  virtual void exitPropertyName(EscriptParser::PropertyNameContext *ctx) = 0;

  virtual void enterTypeAnnotation(EscriptParser::TypeAnnotationContext *ctx) = 0;
  virtual void exitTypeAnnotation(EscriptParser::TypeAnnotationContext *ctx) = 0;

  virtual void enterCallSignature(EscriptParser::CallSignatureContext *ctx) = 0;
  virtual void exitCallSignature(EscriptParser::CallSignatureContext *ctx) = 0;

  virtual void enterParameterList(EscriptParser::ParameterListContext *ctx) = 0;
  virtual void exitParameterList(EscriptParser::ParameterListContext *ctx) = 0;

  virtual void enterParameter(EscriptParser::ParameterContext *ctx) = 0;
  virtual void exitParameter(EscriptParser::ParameterContext *ctx) = 0;

  virtual void enterRestParameter(EscriptParser::RestParameterContext *ctx) = 0;
  virtual void exitRestParameter(EscriptParser::RestParameterContext *ctx) = 0;

  virtual void enterIndexSignature(EscriptParser::IndexSignatureContext *ctx) = 0;
  virtual void exitIndexSignature(EscriptParser::IndexSignatureContext *ctx) = 0;

  virtual void enterMethodSignature(EscriptParser::MethodSignatureContext *ctx) = 0;
  virtual void exitMethodSignature(EscriptParser::MethodSignatureContext *ctx) = 0;


};

}  // namespace EscriptGrammar
