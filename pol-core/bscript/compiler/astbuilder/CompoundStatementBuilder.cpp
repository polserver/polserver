#include "CompoundStatementBuilder.h"

#include "compiler/ast/Expression.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
CompoundStatementBuilder::CompoundStatementBuilder(
    const SourceFileIdentifier& source_file_identifier, BuilderWorkspace& workspace )
    : SimpleStatementBuilder( source_file_identifier, workspace )
{
}

void CompoundStatementBuilder::add_statements(
    EscriptParser::StatementContext* ctx, std::vector<std::unique_ptr<Statement>>& statements )
{
  if ( auto expr_ctx = ctx->expression() )
  {
    statements.push_back( consume_statement_result( expression( expr_ctx ) ) );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled statement" );
  }
}

void CompoundStatementBuilder::add_statements(
    EscriptParser::UnambiguousStatementContext* ctx,
    std::vector<std::unique_ptr<Statement>>& statements )
{
  if ( auto expr_ctx = ctx->unambiguousExpression() )
  {
    statements.push_back( consume_statement_result( expression( expr_ctx ) ) );
  }
  else
  {
    auto source_location = location_for( *ctx );
    source_location.internal_error( "unhandled statement.\n" );
  }
}

}  // namespace Pol::Bscript::Compiler
