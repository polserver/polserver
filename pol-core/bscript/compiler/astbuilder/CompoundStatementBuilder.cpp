#include "CompoundStatementBuilder.h"

#include "compiler/ast/Block.h"
#include "compiler/ast/ExitStatement.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/IfThenElseStatement.h"
#include "compiler/ast/ReturnStatement.h"

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
  else if ( auto if_st = ctx->ifStatement() )
  {
    statements.push_back( if_statement( if_st ) );
  }
  else if ( auto var_statement = ctx->varStatement() )
  {
    add_var_statements( var_statement, statements );
  }
  else if ( auto return_st = ctx->returnStatement() )
  {
    statements.push_back( return_statement( return_st ) );
  }
  else if ( auto exit = ctx->exitStatement() )
  {
    statements.push_back( std::make_unique<ExitStatement>( location_for( *exit ) ) );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled statement" );
  }
}

std::vector<std::unique_ptr<Statement>> CompoundStatementBuilder::block_statements(
        EscriptParser::BlockContext* ctx )
{
  std::vector<std::unique_ptr<Statement>> statements;

  for ( auto statement_context : ctx->statement() )
  {
    add_statements( statement_context, statements );
  }

  return statements;
}

std::unique_ptr<Block> CompoundStatementBuilder::block( EscriptParser::BlockContext* ctx )
{
  auto statements = block_statements( ctx );
  return std::make_unique<Block>( location_for( *ctx ), std::move( statements ) );
}

std::unique_ptr<Statement> CompoundStatementBuilder::if_statement(
    EscriptParser::IfStatementContext* ctx )
{
  //
  // if (expr0) block0 elseif (expr1) block1 ... elseif (exprN-1) blockN-1 else blockN
  //

  auto blocks = ctx->block();
  auto par_expression = ctx->parExpression();

  auto else_clause =
      ctx->ELSE() ? block( blocks.at( blocks.size() - 1 ) ) : std::unique_ptr<Node>();

  auto if_statement_ast = std::unique_ptr<Statement>();

  size_t num_expressions = par_expression.size();
  for ( auto clause_index = num_expressions; clause_index != 0; )
  {
    --clause_index;
    auto expression_ctx = par_expression.at( clause_index );
    auto expression_ast = expression( expression_ctx->expression() );
    auto consequent_ast = block( blocks.at( clause_index ) );
    auto alternative_ast =
        if_statement_ast ? std::move( if_statement_ast ) : std::move( else_clause );
    auto source_location = location_for( *expression_ctx );

    if ( alternative_ast )
    {
      if_statement_ast = std::make_unique<IfThenElseStatement>(
          source_location, std::move( expression_ast ), std::move( consequent_ast ),
          std::move( alternative_ast ) );
    }
    else
    {
      if_statement_ast = std::make_unique<IfThenElseStatement>(
          source_location, std::move( expression_ast ), std::move( consequent_ast ) );
    }
  }

  return if_statement_ast;
}

}  // namespace Pol::Bscript::Compiler
