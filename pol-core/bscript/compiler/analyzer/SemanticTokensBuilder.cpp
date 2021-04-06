#include "SemanticTokensBuilder.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

using namespace EscriptGrammar;
namespace Pol::Bscript::Compiler
{
SemanticTokensBuilder::SemanticTokensBuilder( CompilerWorkspace& workspace )
    : workspace( workspace )
{
}

void SemanticTokensBuilder::build()
{
  if ( workspace.source )
  {
    workspace.tokens = workspace.source->get_tokens();
    workspace.source->accept( *this );
  }
}

void define( CompilerWorkspace& workspace, antlr4::tree::TerminalNode* node,
             SemanticTokenType type )
{
  // Since we no longer fast-fail after parser errors, it is possible that nodes
  // do not exist.
  if ( node )
  {
    if ( auto symbol = node->getSymbol() )
    {
      size_t line = symbol->getLine();
      size_t character = symbol->getCharPositionInLine() + 1;
      size_t token_length = symbol->getText().length();
      workspace.tokens.push_back( SemanticToken{ line, character, token_length, type, {} } );
    }
  }
}

antlrcpp::Any SemanticTokensBuilder::visitModuleFunctionParameter(
    EscriptParser::ModuleFunctionParameterContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::PARAMETER );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitStringIdentifier(
    EscriptParser::StringIdentifierContext* ctx )
{
  // stringIdentifier is currently produced in useDeclaration and
  // includeDeclaration, so this will highlight both "use uo" and "include
  // utils" as namespaces.
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::NAMESPACE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitModuleFunctionDeclaration(
    EscriptParser::ModuleFunctionDeclarationContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitProgramDeclaration(
    EscriptParser::ProgramDeclarationContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitForeachIterableExpression(
    EscriptParser::ForeachIterableExpressionContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitForeachStatement(
    EscriptParser::ForeachStatementContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitEnumStatement( EscriptParser::EnumStatementContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitEnumListEntry( EscriptParser::EnumListEntryContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitSwitchLabel( EscriptParser::SwitchLabelContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitBasicForStatement(
    EscriptParser::BasicForStatementContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitVariableDeclaration(
    EscriptParser::VariableDeclarationContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitProgramParameter(
    EscriptParser::ProgramParameterContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::PARAMETER );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitFunctionParameter(
    EscriptParser::FunctionParameterContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::PARAMETER );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitScopedFunctionCall(
    EscriptParser::ScopedFunctionCallContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::NAMESPACE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitFunctionReference(
    EscriptParser::FunctionReferenceContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitPrimary( EscriptParser::PrimaryContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::VARIABLE );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitNavigationSuffix(
    EscriptParser::NavigationSuffixContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::PROPERTY );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitMethodCallSuffix(
    EscriptParser::MethodCallSuffixContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitFunctionDeclaration(
    EscriptParser::FunctionDeclarationContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}


antlrcpp::Any SemanticTokensBuilder::visitFunctionCall( EscriptParser::FunctionCallContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::FUNCTION );
  visitChildren( ctx );
  return antlrcpp::Any();
}

antlrcpp::Any SemanticTokensBuilder::visitStructInitializerExpression(
    EscriptParser::StructInitializerExpressionContext* ctx )
{
  define( workspace, ctx->IDENTIFIER(), SemanticTokenType::PROPERTY );
  visitChildren( ctx );
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
