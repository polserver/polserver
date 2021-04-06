#ifndef POLSERVER_SEMANTICTOKENSBUILDER_H
#define POLSERVER_SEMANTICTOKENSBUILDER_H

#include "bscript/compiler/model/SemanticTokens.h"
#include <EscriptGrammar/EscriptParserBaseVisitor.h>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Report;

class SemanticTokensBuilder : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SemanticTokensBuilder( CompilerWorkspace& workspace );

  void build();

  antlrcpp::Any visitModuleFunctionDeclaration(
      EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx ) override;
  antlrcpp::Any visitModuleFunctionParameter(
      EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx ) override;
  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx ) override;
  antlrcpp::Any visitStringIdentifier(
      EscriptGrammar::EscriptParser::StringIdentifierContext* ctx ) override;
  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx ) override;
  antlrcpp::Any visitForeachIterableExpression(
      EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* ctx ) override;
  antlrcpp::Any visitForeachStatement(
      EscriptGrammar::EscriptParser::ForeachStatementContext* ctx ) override;
  antlrcpp::Any visitEnumStatement(
      EscriptGrammar::EscriptParser::EnumStatementContext* ctx ) override;
  antlrcpp::Any visitEnumListEntry(
      EscriptGrammar::EscriptParser::EnumListEntryContext* ctx ) override;
  antlrcpp::Any visitSwitchLabel( EscriptGrammar::EscriptParser::SwitchLabelContext* ctx ) override;
  antlrcpp::Any visitBasicForStatement(
      EscriptGrammar::EscriptParser::BasicForStatementContext* ctx ) override;
  antlrcpp::Any visitVariableDeclaration(
      EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx ) override;
  antlrcpp::Any visitProgramParameter(
      EscriptGrammar::EscriptParser::ProgramParameterContext* ctx ) override;
  antlrcpp::Any visitFunctionParameter(
      EscriptGrammar::EscriptParser::FunctionParameterContext* ctx ) override;
  antlrcpp::Any visitScopedFunctionCall(
      EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx ) override;
  antlrcpp::Any visitFunctionReference(
      EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx ) override;
  antlrcpp::Any visitPrimary( EscriptGrammar::EscriptParser::PrimaryContext* ctx ) override;
  antlrcpp::Any visitNavigationSuffix(
      EscriptGrammar::EscriptParser::NavigationSuffixContext* ctx ) override;
  antlrcpp::Any visitMethodCallSuffix(
      EscriptGrammar::EscriptParser::MethodCallSuffixContext* ctx ) override;
  antlrcpp::Any visitFunctionCall(
      EscriptGrammar::EscriptParser::FunctionCallContext* ctx ) override;
  antlrcpp::Any visitStructInitializerExpression(
      EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx ) override;

private:
  CompilerWorkspace& workspace;
};
}  // namespace Pol::Bscript::Compiler

#endif
