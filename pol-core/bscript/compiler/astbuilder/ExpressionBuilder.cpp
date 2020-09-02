#include "ExpressionBuilder.h"

#include "compiler/Report.h"

#include "compiler/ast/Argument.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/astbuilder/BuilderWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

[[noreturn]] BTokenId ExpressionBuilder::unhandled_operator( const SourceLocation& source_location )
{
  // This indicates a log error in the compiler: likely a disconnect between the grammar
  // and the code building the AST from the parsed file.
  source_location.internal_error( "unhandled operator.\n" );
}

std::unique_ptr<BinaryOperator> ExpressionBuilder::binary_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto lhs = expression( ctx->expression( 0 ) );
  auto rhs = expression( ctx->expression( 1 ) );

  BTokenId token_id;
  if ( ctx->ADD() )
    token_id = TOK_ADD;
  else if ( ctx->SUB() )
    token_id = TOK_SUBTRACT;
  else if ( ctx->MUL() )
    token_id = TOK_MULT;
  else if ( ctx->DIV() )
    token_id = TOK_DIV;
  else if ( ctx->ASSIGN() )
    token_id = TOK_ASSIGN;
  else if ( ctx->EQUAL() )
    token_id = TOK_EQUAL;
  else if ( ctx->NOTEQUAL_A() || ctx->NOTEQUAL_B() )
    token_id = TOK_NEQ;
  else if ( ctx->LT() )
    token_id = TOK_LESSTHAN;
  else if ( ctx->LE() )
    token_id = TOK_LESSEQ;
  else if ( ctx->GT() )
    token_id = TOK_GRTHAN;
  else if ( ctx->GE() )
    token_id = TOK_GREQ;
  else if ( ctx->AND_A() || ctx->AND_B() )
    token_id = TOK_AND;
  else if ( ctx->OR_A() || ctx->OR_B() )
    token_id = TOK_OR;
  else if ( ctx->ADD_ASSIGN() )
    token_id = TOK_PLUSEQUAL;
  else if ( ctx->SUB_ASSIGN() )
    token_id = TOK_MINUSEQUAL;
  else if ( ctx->MUL_ASSIGN() )
    token_id = TOK_TIMESEQUAL;
  else if ( ctx->DIV_ASSIGN() )
    token_id = TOK_DIVIDEEQUAL;
  else if ( ctx->MOD() )
    token_id = TOK_MODULUS;
  else if ( ctx->MOD_ASSIGN() )
    token_id = TOK_MODULUSEQUAL;
  else if ( ctx->ADDMEMBER() )
    token_id = TOK_ADDMEMBER;
  else if ( ctx->DELMEMBER() )
    token_id = TOK_DELMEMBER;
  else if ( ctx->CHKMEMBER() )
    token_id = TOK_CHKMEMBER;
  else if ( ctx->BITAND() )
    token_id = TOK_BITAND;
  else if ( ctx->BITOR() )
    token_id = TOK_BITOR;
  else if ( ctx->CARET() )
    token_id = TOK_BITXOR;
  else if ( ctx->TOK_IN() )
    token_id = TOK_IN;
  else if ( ctx->LSHIFT() )
    token_id = TOK_BSLEFT;
  else if ( ctx->RSHIFT() )
    token_id = TOK_BSRIGHT;
  else
    location_for( *ctx ).internal_error( "unrecognized binary operator" );
  return std::make_unique<BinaryOperator>( location_for( *ctx ), std::move( lhs ),
                                           ctx->bop->getText(), token_id, std::move( rhs ) );
}

std::unique_ptr<Expression> ExpressionBuilder::expression( EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return primary( prim );
  else if ( ctx->prefix )
    return prefix_unary_operator( ctx );
  else if ( ctx->postfix )
    return postfix_unary_operator( ctx );
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    return binary_operator( ctx );
  }

  location_for( *ctx ).internal_error( "unhandled expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::FunctionCallContext* ctx, const std::string& scope )
{
  auto method_name = text( ctx->IDENTIFIER() );

  auto arguments = value_arguments( ctx->expressionList() );

  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, method_name,
                                                       std::move( arguments ) );

  std::string key = scope.empty() ? method_name : ( scope + "::" + method_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto expression_ctx = ctx->expression( 0 );
  auto source_location = location_for( *expression_ctx );
  auto expression_ast = expression( expression_ctx );

  BTokenId token_id =
      ctx->ADD()
          ? TOK_UNPLUS
          : ctx->SUB()
                ? TOK_UNMINUS
                : ctx->INC()
                      ? TOK_UNPLUSPLUS
                      : ctx->DEC() ? TOK_UNMINUSMINUS
                                   : ctx->TILDE() ? TOK_BITWISE_NOT
                                                  : ( ctx->BANG_A() || ctx->BANG_B() )
                                                        ? TOK_LOG_NOT
                                                        : unhandled_operator( source_location );

  if ( token_id == TOK_UNPLUS )
    return expression_ast;

  return std::make_unique<UnaryOperator>( source_location, ctx->prefix->getText(), token_id,
                                          std::move( expression_ast ) );
}

std::unique_ptr<Expression> ExpressionBuilder::postfix_unary_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto loc = location_for( *ctx );
  auto expression_ctx = ctx->expression( 0 );
  auto expression_ast = expression( expression_ctx );

  BTokenId token_id = ctx->INC() ? TOK_UNPLUSPLUS_POST
                                 : ctx->DEC() ? TOK_UNMINUSMINUS_POST : unhandled_operator( loc );

  return std::make_unique<UnaryOperator>( loc, ctx->postfix->getText(), token_id,
                                          std::move( expression_ast ) );
}

std::unique_ptr<Expression> ExpressionBuilder::primary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
  {
    return value( literal );
  }
  else if ( auto par_expression = ctx->parExpression() )
  {
    return expression( par_expression->expression() );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return std::make_unique<Identifier>( location_for( *ctx ), text( identifier ) );
  }
  else if ( auto f_call = ctx->functionCall() )
  {
    return function_call( f_call, "" );
  }
  else if ( auto scoped_f_call = ctx->scopedFunctionCall() )
  {
    return scoped_function_call( scoped_f_call );
  }

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::ScopedFunctionCallContext* ctx )
{
  return function_call( ctx->functionCall(), text( ctx->IDENTIFIER() ) );
}

std::vector<std::unique_ptr<Argument>> ExpressionBuilder::value_arguments(
    EscriptGrammar::EscriptParser::ExpressionListContext* ctx )
{
  std::vector<std::unique_ptr<Argument>> arguments;

  if ( ctx )
  {
    for ( auto argument_context : ctx->expression() )
    {
      auto loc = location_for( *argument_context );

      std::string name;
      auto value = expression( argument_context );

      if ( auto binary_operator = dynamic_cast<BinaryOperator*>( value.get() ) )
      {
        if ( binary_operator->token_id == TOK_ASSIGN )
        {
          if ( auto identifier = dynamic_cast<Identifier*>( &binary_operator->lhs() ) )
          {
            name = identifier->name;
            value = binary_operator->take_rhs();
          }
        }
      }
      auto argument = std::make_unique<Argument>( loc, std::move( name ), std::move( value ) );
      arguments.push_back( std::move( argument ) );
    }
  }

  return arguments;
}

}  // namespace Pol::Bscript::Compiler
