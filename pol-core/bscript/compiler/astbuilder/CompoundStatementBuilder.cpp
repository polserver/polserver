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
  else if ( auto var_statement = ctx->varStatement() )
  {
    add_var_statements( var_statement, statements );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled statement" );
  }
}

}  // namespace Pol::Bscript::Compiler
