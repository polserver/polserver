#include "UserFunctionVisitor.h"

#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
UserFunctionVisitor::UserFunctionVisitor( const SourceFileIdentifier& sfi, BuilderWorkspace& ws )
  : workspace( ws ), tree_builder( sfi, ws )
{
}

antlrcpp::Any UserFunctionVisitor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  auto uf = tree_builder.function_declaration( ctx );
  workspace.function_resolver.register_user_function( uf.get() );
  workspace.compiler_workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

antlrcpp::Any UserFunctionVisitor::visitFunctionExpression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto uf = tree_builder.function_expression( ctx );
  workspace.function_resolver.register_user_function( uf.get() );
  workspace.compiler_workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
