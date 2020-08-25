#include "SimpleStatementBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/model/CompilerWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
SimpleStatementBuilder::SimpleStatementBuilder( const SourceFileIdentifier& source_file_identifier,
                                    BuilderWorkspace& workspace )
  : ExpressionBuilder( source_file_identifier, workspace )
{
}

void SimpleStatementBuilder::add_var_statements(
    EscriptParser::VarStatementContext* ctx, std::vector<std::unique_ptr<Statement>>& statements )
{
  if ( auto variable_declaration_list = ctx->variableDeclarationList() )
  {
    for ( auto decl : variable_declaration_list->variableDeclaration() )
    {
      auto loc = location_for( *decl );
      std::string name = text( decl->IDENTIFIER() );
      std::unique_ptr<VarStatement> var_ast;

      if ( auto initializer_context = decl->variableDeclarationInitializer() )
      {
        if ( initializer_context->ARRAY() )
        {
          var_ast = std::make_unique<VarStatement>( loc, std::move( name ), true );
        }
        else
        {
          auto initializer = variable_initializer( initializer_context );
          var_ast =
              std::make_unique<VarStatement>( loc, std::move( name ), std::move( initializer ) );
        }
      }
      else
      {
        var_ast = std::make_unique<VarStatement>( loc, std::move( name ) );
      }
      auto consumed = consume_statement_result( std::move( var_ast ) );
      statements.push_back( std::move( consumed ) );
    }
  }
}

std::unique_ptr<Statement> SimpleStatementBuilder::consume_statement_result(
    std::unique_ptr<Statement> statement )
{
  return std::make_unique<ValueConsumer>( statement->source_location, std::move( statement ) );
}

std::unique_ptr<Expression> SimpleStatementBuilder::variable_initializer(
    EscriptParser::VariableDeclarationInitializerContext* ctx )
{
  if ( auto expr = ctx->expression() )
    return expression( expr );
  else
    return std::unique_ptr<Expression>( new StringValue( location_for( *ctx ), "" ) );
}

}  // namespace Pol::Bscript::Compiler
