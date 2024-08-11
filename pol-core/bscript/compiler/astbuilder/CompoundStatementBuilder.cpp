#include "CompoundStatementBuilder.h"

#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/BasicForLoop.h"
#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/BranchSelector.h"
#include "bscript/compiler/ast/CaseDispatchDefaultSelector.h"
#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/CaseDispatchGroups.h"
#include "bscript/compiler/ast/CaseDispatchSelectors.h"
#include "bscript/compiler/ast/CaseStatement.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DebugStatementMarker.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/EmptyStatement.h"
#include "bscript/compiler/ast/EnumDeclaration.h"
#include "bscript/compiler/ast/ExitStatement.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IfThenElseStatement.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/RepeatUntilLoop.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/ast/WhileLoop.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/ScopeName.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript
{
extern int include_debug;
}

namespace Pol::Bscript::Compiler
{
CompoundStatementBuilder::CompoundStatementBuilder(
    const SourceFileIdentifier& source_file_identifier, BuilderWorkspace& workspace )
    : SimpleStatementBuilder( source_file_identifier, workspace )
{
}

void CompoundStatementBuilder::add_statements( EscriptParser::StatementContext* ctx,
                                               std::vector<std::unique_ptr<Statement>>& statements )
{
  if ( include_debug )
  {
    add_intrusive_debug_marker( ctx, statements );
  }

  if ( auto expr_ctx = ctx->expression() )
  {
    statements.push_back( expression( expr_ctx, true ) );
  }
  else if ( auto if_st = ctx->ifStatement() )
  {
    statements.push_back( if_statement( if_st ) );
  }
  else if ( auto var_statement = ctx->varStatement() )
  {
    add_var_statements( var_statement, "", statements );
  }
  else if ( auto return_st = ctx->returnStatement() )
  {
    statements.push_back( return_statement( return_st ) );
  }
  else if ( auto foreach_statement = ctx->foreachStatement() )
  {
    statements.push_back( foreach_loop( foreach_statement ) );
  }
  else if ( auto for_st = ctx->forStatement() )
  {
    std::string label;
    if ( auto statement_label = for_st->statementLabel() )
      label = text( statement_label->IDENTIFIER() );
    statements.push_back( for_loop( for_st->forGroup(), std::move( label ) ) );
  }
  else if ( auto while_statement = ctx->whileStatement() )
  {
    statements.push_back( while_loop( while_statement ) );
  }
  else if ( auto break_st = ctx->breakStatement() )
  {
    statements.push_back( break_statement( break_st ) );
  }
  else if ( auto continue_st = ctx->continueStatement() )
  {
    statements.push_back( continue_statement( continue_st ) );
  }
  else if ( auto enum_statement = ctx->enumStatement() )
  {
    statements.push_back( enum_declaration( enum_statement ) );
  }
  else if ( auto case_st = ctx->caseStatement() )
  {
    statements.push_back( case_statement( case_st ) );
  }
  else if ( auto repeat_statement = ctx->repeatStatement() )
  {
    statements.push_back( repeat_until_loop( repeat_statement ) );
  }
  else if ( auto exit = ctx->exitStatement() )
  {
    statements.push_back( std::make_unique<ExitStatement>( location_for( *exit ) ) );
  }
  else if ( auto const_statement = ctx->constStatement() )
  {
    workspace.compiler_workspace.const_declarations.push_back(
        const_declaration( const_statement ) );
  }
  else if ( auto do_statement = ctx->doStatement() )
  {
    statements.push_back( do_while_loop( do_statement ) );
  }
  else if ( ctx->SEMI() )
  {
    statements.push_back( std::make_unique<EmptyStatement>( location_for( *ctx ) ) );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled statement" );
  }
}

std::unique_ptr<BasicForLoop> CompoundStatementBuilder::basic_for_loop(
    EscriptParser::BasicForStatementContext* ctx, std::string label )
{
  auto identifier = text( ctx->IDENTIFIER() );
  auto first = expression( ctx->expression( 0 ) );
  auto last = expression( ctx->expression( 1 ) );
  auto body = block( ctx->block() );

  return std::make_unique<BasicForLoop>( location_for( *ctx ), std::move( label ),
                                         std::move( identifier ), std::move( first ),
                                         std::move( last ), std::move( body ) );
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

std::unique_ptr<CaseStatement> CompoundStatementBuilder::case_statement(
    EscriptParser::CaseStatementContext* ctx )
{
  // possibilities:
  // case (expr)
  //  4:
  //  "xz":
  //  CONSTANT:
  //  default:
  auto loc = location_for( *ctx );
  std::string case_label;
  if ( auto statement_label = ctx->statementLabel() )
    case_label = text( statement_label->IDENTIFIER() );

  auto determinant = expression( ctx->expression() );

  std::vector<std::unique_ptr<CaseDispatchGroup>> dispatch_groups;

  for ( auto group : ctx->switchBlockStatementGroup() )
  {
    std::vector<std::unique_ptr<Node>> selectors;

    for ( auto group_label : group->switchLabel() )
    {
      if ( auto integer_literal = group_label->integerLiteral() )
      {
        selectors.push_back( integer_value( integer_literal ) );
      }
      else if ( auto identifier = group_label->IDENTIFIER() )
      {
        selectors.push_back(
            std::make_unique<Identifier>( location_for( *identifier ), text( identifier ) ) );
      }
      else if ( auto string_literal = group_label->STRING_LITERAL() )
      {
        selectors.push_back( string_value( string_literal ) );
      }
      if ( auto bool_literal = group_label->boolLiteral() )
      {
        selectors.push_back( bool_value( bool_literal ) );
      }
      if ( auto uninit = group_label->UNINIT() )
      {
        selectors.push_back( std::make_unique<UninitializedValue>( location_for( *uninit ) ) );
      }
      else if ( auto dflt = group_label->DEFAULT() )
      {
        selectors.push_back(
            std::make_unique<CaseDispatchDefaultSelector>( location_for( *dflt ) ) );
      }
    }
    auto dispatch_selectors =
        std::make_unique<CaseDispatchSelectors>( location_for( *group ), std::move( selectors ) );
    auto body = block( group->block() );
    auto dispatch_group = std::make_unique<CaseDispatchGroup>(
        location_for( *group ), std::move( dispatch_selectors ), std::move( body ) );
    dispatch_groups.push_back( std::move( dispatch_group ) );
  }
  auto holder = std::make_unique<CaseDispatchGroups>( loc, std::move( dispatch_groups ) );

  return std::make_unique<CaseStatement>( loc, std::move( case_label ), std::move( determinant ),
                                          std::move( holder ) );
}

std::unique_ptr<CstyleForLoop> CompoundStatementBuilder::cstyle_for_loop(
    EscriptParser::CstyleForStatementContext* ctx, std::string label )
{
  auto initializer = expression( ctx->expression( 0 ) );
  auto predicate = expression( ctx->expression( 1 ) );
  auto advancer = expression( ctx->expression( 2 ) );
  auto body = block( ctx->block() );
  return std::make_unique<CstyleForLoop>( location_for( *ctx ), std::move( label ),
                                          std::move( initializer ), std::move( predicate ),
                                          std::move( advancer ), std::move( body ) );
}

std::unique_ptr<DoWhileLoop> CompoundStatementBuilder::do_while_loop(
    EscriptParser::DoStatementContext* ctx )
{
  auto source_location = location_for( *ctx );
  std::string label;
  if ( auto statement_label = ctx->statementLabel() )
    label = text( statement_label->IDENTIFIER() );
  auto body = block( ctx->block() );
  auto predicate = expression( ctx->parExpression()->expression() );
  return std::make_unique<DoWhileLoop>( source_location, std::move( label ), std::move( body ),
                                        std::move( predicate ) );
}

std::unique_ptr<Statement> CompoundStatementBuilder::for_loop( EscriptParser::ForGroupContext* ctx,
                                                               std::string label )
{
  if ( auto cstyle = ctx->cstyleForStatement() )
  {
    return cstyle_for_loop( cstyle, std::move( label ) );
  }
  else if ( auto basic = ctx->basicForStatement() )
  {
    return basic_for_loop( basic, std::move( label ) );
  }
  else
  {
    location_for( *ctx ).internal_error(
        "neither c-style nor basic-style for statement in for group" );
  }
}

std::unique_ptr<Expression> CompoundStatementBuilder::foreach_iterable_expression(
    EscriptParser::ForeachIterableExpressionContext* ctx )
{
  if ( auto scoped_ident = ctx->scopedIdentifier() )
  {
    return scoped_identifier( scoped_ident );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return std::make_unique<Identifier>( location_for( *identifier ), text( identifier ) );
  }
  else if ( auto m_call = ctx->functionCall() )
  {
    return function_call( m_call, ScopeName::None );
  }
  else if ( auto scoped_call = ctx->scopedFunctionCall() )
  {
    return scoped_function_call( scoped_call );
  }
  else if ( auto array_init = ctx->explicitArrayInitializer() )
  {
    return array_initializer( array_init );
  }
  else if ( auto bare_array = ctx->bareArrayInitializer() )
  {
    return array_initializer( bare_array );
  }
  else if ( auto par_ex = ctx->parExpression() )
  {
    return expression( par_ex->expression() );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled foreach iterable expression" );
  }
}

std::unique_ptr<ForeachLoop> CompoundStatementBuilder::foreach_loop(
    EscriptParser::ForeachStatementContext* ctx )
{
  auto source_location = location_for( *ctx );
  std::string label;
  if ( auto statement_label = ctx->statementLabel() )
    label = text( statement_label->IDENTIFIER() );
  std::string iterator_name = text( ctx->IDENTIFIER() );
  auto iterable = foreach_iterable_expression( ctx->foreachIterableExpression() );
  auto body = block( ctx->block() );
  return std::make_unique<ForeachLoop>( source_location, std::move( label ),
                                        std::move( iterator_name ), std::move( iterable ),
                                        std::move( body ) );
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

    auto branch_selector = std::make_unique<BranchSelector>(
        source_location, BranchSelector::IfFalse, std::move( expression_ast ) );

    if ( alternative_ast )
    {
      if_statement_ast = std::make_unique<IfThenElseStatement>(
          source_location, std::move( branch_selector ), std::move( consequent_ast ),
          std::move( alternative_ast ) );
    }
    else
    {
      if_statement_ast = std::make_unique<IfThenElseStatement>(
          source_location, std::move( branch_selector ), std::move( consequent_ast ) );
    }
  }

  return if_statement_ast;
}

std::unique_ptr<RepeatUntilLoop> CompoundStatementBuilder::repeat_until_loop(
    EscriptParser::RepeatStatementContext* ctx )
{
  auto source_location = location_for( *ctx );
  std::string label;
  if ( auto statement_label = ctx->statementLabel() )
    label = text( statement_label->IDENTIFIER() );
  auto body = block( ctx->block() );
  auto predicate = expression( ctx->expression() );
  return std::make_unique<RepeatUntilLoop>( source_location, std::move( label ), std::move( body ),
                                            std::move( predicate ) );
}

std::unique_ptr<WhileLoop> CompoundStatementBuilder::while_loop(
    EscriptParser::WhileStatementContext* ctx )
{
  std::string label;
  if ( auto statement_label = ctx->statementLabel() )
    label = text( statement_label->IDENTIFIER() );
  auto source_location = location_for( *ctx );
  auto predicate = expression( ctx->parExpression()->expression() );
  auto body = block( ctx->block() );
  return std::make_unique<WhileLoop>( source_location, std::move( label ), std::move( predicate ),
                                      std::move( body ) );
}

}  // namespace Pol::Bscript::Compiler
