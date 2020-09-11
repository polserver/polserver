#include "UserFunctionVisitor.h"

#include "compiler/ast/UserFunction.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/model/CompilerWorkspace.h"

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
  workspace.compiler_workspace.all_function_locations.emplace(uf->name, uf->source_location);
  workspace.compiler_workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
