#include "UserFunctionVisitor.h"

#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/DefaultConstructorFunction.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
UserFunctionVisitor::UserFunctionVisitor( const SourceFileIdentifier& sfi, BuilderWorkspace& ws,
                                          const std::string& scope )
    : workspace( ws ), tree_builder( sfi, ws ), scope( scope )
{
}

antlrcpp::Any UserFunctionVisitor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  if ( !scope.empty() )
  {
    tree_builder.current_scope_name = ScopeName( scope );
  }

  auto uf = tree_builder.function_declaration( ctx, scope );
  workspace.function_resolver.register_user_function( scope, uf.get() );
  if ( uf->type == UserFunctionType::Constructor )
    workspace.function_resolver.register_user_function( "", uf.get() );

  workspace.compiler_workspace.user_functions.push_back( std::move( uf ) );

  if ( !scope.empty() )
  {
    tree_builder.current_scope_name = ScopeName::Global;
  }

  return antlrcpp::Any();
}

antlrcpp::Any UserFunctionVisitor::visitFunctionExpression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto uf = tree_builder.function_expression( ctx );
  workspace.function_resolver.register_user_function( "", uf.get() );
  workspace.compiler_workspace.user_functions.push_back( std::move( uf ) );
  return antlrcpp::Any();
}

antlrcpp::Any UserFunctionVisitor::visitClassDeclaration(
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx )
{
  auto cd = tree_builder.class_declaration( ctx );
  workspace.compiler_workspace.class_declarations.push_back( std::move( cd ) );
  return antlrcpp::Any();
}

}  // namespace Pol::Bscript::Compiler
