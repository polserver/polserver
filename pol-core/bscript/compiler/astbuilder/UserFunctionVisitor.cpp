#include "UserFunctionVisitor.h"

#include "BuilderWorkspace.h"
#include "compiler/ast/UserFunction.h"

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
  workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

antlrcpp::Any UserFunctionVisitor::visitUnambiguousFunctionDeclaration(
    EscriptGrammar::EscriptParser::UnambiguousFunctionDeclarationContext* ctx )
{
  auto uf = tree_builder.function_declaration( ctx );
  workspace.function_resolver.register_user_function( uf.get() );
  workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
