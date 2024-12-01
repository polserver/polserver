


// Generated from EscriptParser.g4 by ANTLR 4.13.1

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
    virtual std::any visitCompilationUnit(EscriptParser::CompilationUnitContext *context) = 0;

    virtual std::any visitModuleUnit(EscriptParser::ModuleUnitContext *context) = 0;

    virtual std::any visitEvaluateUnit(EscriptParser::EvaluateUnitContext *context) = 0;

    virtual std::any visitModuleDeclarationStatement(EscriptParser::ModuleDeclarationStatementContext *context) = 0;

    virtual std::any visitModuleFunctionDeclaration(EscriptParser::ModuleFunctionDeclarationContext *context) = 0;

    virtual std::any visitModuleFunctionParameterList(EscriptParser::ModuleFunctionParameterListContext *context) = 0;

    virtual std::any visitModuleFunctionParameter(EscriptParser::ModuleFunctionParameterContext *context) = 0;

    virtual std::any visitTopLevelDeclaration(EscriptParser::TopLevelDeclarationContext *context) = 0;

    virtual std::any visitClassDeclaration(EscriptParser::ClassDeclarationContext *context) = 0;

    virtual std::any visitClassParameters(EscriptParser::ClassParametersContext *context) = 0;

    virtual std::any visitClassParameterList(EscriptParser::ClassParameterListContext *context) = 0;

    virtual std::any visitClassBody(EscriptParser::ClassBodyContext *context) = 0;

    virtual std::any visitClassStatement(EscriptParser::ClassStatementContext *context) = 0;

    virtual std::any visitFunctionDeclaration(EscriptParser::FunctionDeclarationContext *context) = 0;

    virtual std::any visitStringIdentifier(EscriptParser::StringIdentifierContext *context) = 0;

    virtual std::any visitUseDeclaration(EscriptParser::UseDeclarationContext *context) = 0;

    virtual std::any visitIncludeDeclaration(EscriptParser::IncludeDeclarationContext *context) = 0;

    virtual std::any visitProgramDeclaration(EscriptParser::ProgramDeclarationContext *context) = 0;

    virtual std::any visitStatement(EscriptParser::StatementContext *context) = 0;

    virtual std::any visitStatementLabel(EscriptParser::StatementLabelContext *context) = 0;

    virtual std::any visitIfStatement(EscriptParser::IfStatementContext *context) = 0;

    virtual std::any visitGotoStatement(EscriptParser::GotoStatementContext *context) = 0;

    virtual std::any visitReturnStatement(EscriptParser::ReturnStatementContext *context) = 0;

    virtual std::any visitConstStatement(EscriptParser::ConstStatementContext *context) = 0;

    virtual std::any visitVarStatement(EscriptParser::VarStatementContext *context) = 0;

    virtual std::any visitDoStatement(EscriptParser::DoStatementContext *context) = 0;

    virtual std::any visitWhileStatement(EscriptParser::WhileStatementContext *context) = 0;

    virtual std::any visitExitStatement(EscriptParser::ExitStatementContext *context) = 0;

    virtual std::any visitBreakStatement(EscriptParser::BreakStatementContext *context) = 0;

    virtual std::any visitContinueStatement(EscriptParser::ContinueStatementContext *context) = 0;

    virtual std::any visitForStatement(EscriptParser::ForStatementContext *context) = 0;

    virtual std::any visitForeachIterableExpression(EscriptParser::ForeachIterableExpressionContext *context) = 0;

    virtual std::any visitForeachStatement(EscriptParser::ForeachStatementContext *context) = 0;

    virtual std::any visitRepeatStatement(EscriptParser::RepeatStatementContext *context) = 0;

    virtual std::any visitCaseStatement(EscriptParser::CaseStatementContext *context) = 0;

    virtual std::any visitEnumStatement(EscriptParser::EnumStatementContext *context) = 0;

    virtual std::any visitBlock(EscriptParser::BlockContext *context) = 0;

    virtual std::any visitVariableDeclarationInitializer(EscriptParser::VariableDeclarationInitializerContext *context) = 0;

    virtual std::any visitEnumList(EscriptParser::EnumListContext *context) = 0;

    virtual std::any visitEnumListEntry(EscriptParser::EnumListEntryContext *context) = 0;

    virtual std::any visitSwitchBlockStatementGroup(EscriptParser::SwitchBlockStatementGroupContext *context) = 0;

    virtual std::any visitSwitchLabel(EscriptParser::SwitchLabelContext *context) = 0;

    virtual std::any visitForGroup(EscriptParser::ForGroupContext *context) = 0;

    virtual std::any visitBasicForStatement(EscriptParser::BasicForStatementContext *context) = 0;

    virtual std::any visitCstyleForStatement(EscriptParser::CstyleForStatementContext *context) = 0;

    virtual std::any visitIdentifierList(EscriptParser::IdentifierListContext *context) = 0;

    virtual std::any visitVariableDeclarationList(EscriptParser::VariableDeclarationListContext *context) = 0;

    virtual std::any visitConstantDeclaration(EscriptParser::ConstantDeclarationContext *context) = 0;

    virtual std::any visitVariableDeclaration(EscriptParser::VariableDeclarationContext *context) = 0;

    virtual std::any visitProgramParameters(EscriptParser::ProgramParametersContext *context) = 0;

    virtual std::any visitProgramParameterList(EscriptParser::ProgramParameterListContext *context) = 0;

    virtual std::any visitProgramParameter(EscriptParser::ProgramParameterContext *context) = 0;

    virtual std::any visitFunctionParameters(EscriptParser::FunctionParametersContext *context) = 0;

    virtual std::any visitFunctionParameterList(EscriptParser::FunctionParameterListContext *context) = 0;

    virtual std::any visitFunctionParameter(EscriptParser::FunctionParameterContext *context) = 0;

    virtual std::any visitScopedFunctionCall(EscriptParser::ScopedFunctionCallContext *context) = 0;

    virtual std::any visitFunctionReference(EscriptParser::FunctionReferenceContext *context) = 0;

    virtual std::any visitExpression(EscriptParser::ExpressionContext *context) = 0;

    virtual std::any visitPrimary(EscriptParser::PrimaryContext *context) = 0;

    virtual std::any visitScopedIdentifier(EscriptParser::ScopedIdentifierContext *context) = 0;

    virtual std::any visitFunctionExpression(EscriptParser::FunctionExpressionContext *context) = 0;

    virtual std::any visitExplicitArrayInitializer(EscriptParser::ExplicitArrayInitializerContext *context) = 0;

    virtual std::any visitExplicitStructInitializer(EscriptParser::ExplicitStructInitializerContext *context) = 0;

    virtual std::any visitExplicitDictInitializer(EscriptParser::ExplicitDictInitializerContext *context) = 0;

    virtual std::any visitExplicitErrorInitializer(EscriptParser::ExplicitErrorInitializerContext *context) = 0;

    virtual std::any visitBareArrayInitializer(EscriptParser::BareArrayInitializerContext *context) = 0;

    virtual std::any visitParExpression(EscriptParser::ParExpressionContext *context) = 0;

    virtual std::any visitExpressionList(EscriptParser::ExpressionListContext *context) = 0;

    virtual std::any visitExpressionListEntry(EscriptParser::ExpressionListEntryContext *context) = 0;

    virtual std::any visitExpressionSuffix(EscriptParser::ExpressionSuffixContext *context) = 0;

    virtual std::any visitIndexingSuffix(EscriptParser::IndexingSuffixContext *context) = 0;

    virtual std::any visitIndexList(EscriptParser::IndexListContext *context) = 0;

    virtual std::any visitNavigationSuffix(EscriptParser::NavigationSuffixContext *context) = 0;

    virtual std::any visitMethodCallSuffix(EscriptParser::MethodCallSuffixContext *context) = 0;

    virtual std::any visitFunctionCallSuffix(EscriptParser::FunctionCallSuffixContext *context) = 0;

    virtual std::any visitFunctionCall(EscriptParser::FunctionCallContext *context) = 0;

    virtual std::any visitStructInitializerExpression(EscriptParser::StructInitializerExpressionContext *context) = 0;

    virtual std::any visitStructInitializerExpressionList(EscriptParser::StructInitializerExpressionListContext *context) = 0;

    virtual std::any visitStructInitializer(EscriptParser::StructInitializerContext *context) = 0;

    virtual std::any visitDictInitializerExpression(EscriptParser::DictInitializerExpressionContext *context) = 0;

    virtual std::any visitDictInitializerExpressionList(EscriptParser::DictInitializerExpressionListContext *context) = 0;

    virtual std::any visitDictInitializer(EscriptParser::DictInitializerContext *context) = 0;

    virtual std::any visitArrayInitializer(EscriptParser::ArrayInitializerContext *context) = 0;

    virtual std::any visitLiteral(EscriptParser::LiteralContext *context) = 0;

    virtual std::any visitInterpolatedString(EscriptParser::InterpolatedStringContext *context) = 0;

    virtual std::any visitInterpolatedStringPart(EscriptParser::InterpolatedStringPartContext *context) = 0;

    virtual std::any visitIntegerLiteral(EscriptParser::IntegerLiteralContext *context) = 0;

    virtual std::any visitFloatLiteral(EscriptParser::FloatLiteralContext *context) = 0;

    virtual std::any visitBoolLiteral(EscriptParser::BoolLiteralContext *context) = 0;

    virtual std::any visitTypeParameters(EscriptParser::TypeParametersContext *context) = 0;

    virtual std::any visitTypeParameterList(EscriptParser::TypeParameterListContext *context) = 0;

    virtual std::any visitTypeParameter(EscriptParser::TypeParameterContext *context) = 0;

    virtual std::any visitTypeArguments(EscriptParser::TypeArgumentsContext *context) = 0;

    virtual std::any visitTypeArgumentList(EscriptParser::TypeArgumentListContext *context) = 0;

    virtual std::any visitTypeArgument(EscriptParser::TypeArgumentContext *context) = 0;

    virtual std::any visitType_(EscriptParser::Type_Context *context) = 0;

    virtual std::any visitUnionOrIntersectionOrPrimaryType(EscriptParser::UnionOrIntersectionOrPrimaryTypeContext *context) = 0;

    virtual std::any visitPredefinedPrimType(EscriptParser::PredefinedPrimTypeContext *context) = 0;

    virtual std::any visitArrayPrimType(EscriptParser::ArrayPrimTypeContext *context) = 0;

    virtual std::any visitParenthesizedPrimType(EscriptParser::ParenthesizedPrimTypeContext *context) = 0;

    virtual std::any visitTuplePrimType(EscriptParser::TuplePrimTypeContext *context) = 0;

    virtual std::any visitObjectPrimType(EscriptParser::ObjectPrimTypeContext *context) = 0;

    virtual std::any visitReferencePrimType(EscriptParser::ReferencePrimTypeContext *context) = 0;

    virtual std::any visitPredefinedType(EscriptParser::PredefinedTypeContext *context) = 0;

    virtual std::any visitTypeReference(EscriptParser::TypeReferenceContext *context) = 0;

    virtual std::any visitTypeGeneric(EscriptParser::TypeGenericContext *context) = 0;

    virtual std::any visitTypeName(EscriptParser::TypeNameContext *context) = 0;

    virtual std::any visitObjectType(EscriptParser::ObjectTypeContext *context) = 0;

    virtual std::any visitTypeBody(EscriptParser::TypeBodyContext *context) = 0;

    virtual std::any visitTypeMemberList(EscriptParser::TypeMemberListContext *context) = 0;

    virtual std::any visitTypeMember(EscriptParser::TypeMemberContext *context) = 0;

    virtual std::any visitArrayType(EscriptParser::ArrayTypeContext *context) = 0;

    virtual std::any visitTupleType(EscriptParser::TupleTypeContext *context) = 0;

    virtual std::any visitTupleElementTypes(EscriptParser::TupleElementTypesContext *context) = 0;

    virtual std::any visitFunctionType(EscriptParser::FunctionTypeContext *context) = 0;

    virtual std::any visitTypeQueryExpression(EscriptParser::TypeQueryExpressionContext *context) = 0;

    virtual std::any visitIdentifierName(EscriptParser::IdentifierNameContext *context) = 0;

    virtual std::any visitReservedWord(EscriptParser::ReservedWordContext *context) = 0;

    virtual std::any visitPropertySignatur(EscriptParser::PropertySignaturContext *context) = 0;

    virtual std::any visitPropertyName(EscriptParser::PropertyNameContext *context) = 0;

    virtual std::any visitTypeAnnotation(EscriptParser::TypeAnnotationContext *context) = 0;

    virtual std::any visitCallSignature(EscriptParser::CallSignatureContext *context) = 0;

    virtual std::any visitParameterList(EscriptParser::ParameterListContext *context) = 0;

    virtual std::any visitRequiredParameterList(EscriptParser::RequiredParameterListContext *context) = 0;

    virtual std::any visitParameter(EscriptParser::ParameterContext *context) = 0;

    virtual std::any visitOptionalParameter(EscriptParser::OptionalParameterContext *context) = 0;

    virtual std::any visitRestParameter(EscriptParser::RestParameterContext *context) = 0;

    virtual std::any visitRequiredParameter(EscriptParser::RequiredParameterContext *context) = 0;

    virtual std::any visitIndexSignature(EscriptParser::IndexSignatureContext *context) = 0;

    virtual std::any visitMethodSignature(EscriptParser::MethodSignatureContext *context) = 0;


};

}  // namespace EscriptGrammar
