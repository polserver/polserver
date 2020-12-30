#include "ExpressionBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/DictionaryEntry.h"
#include "bscript/compiler/ast/DictionaryInitializer.h"
#include "bscript/compiler/ast/ElementAccess.h"
#include "bscript/compiler/ast/ElementAssignment.h"
#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/ElvisOperator.h"
#include "bscript/compiler/ast/ErrorInitializer.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/MethodCall.h"
#include "bscript/compiler/ast/MethodCallArgumentList.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/StructInitializer.h"
#include "bscript/compiler/ast/StructMemberInitializer.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

void ExpressionBuilder::unhandled_operator( const SourceLocation& source_location )
{
  // This indicates a log error in the compiler: likely a disconnect between the grammar
  // and the code building the AST from the parsed file.
  source_location.internal_error( "unhandled operator.\n" );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::BareArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->expressionList() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->arrayInitializer() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<Expression> ExpressionBuilder::binary_operator(
    EscriptParser::ExpressionContext* ctx, bool consume )
{
  auto lhs = expression( ctx->expression( 0 ) );
  auto rhs = expression( ctx->expression( 1 ) );

  BTokenId token_id = binary_operator_token( ctx );

  if ( token_id == TOK_ASSIGN)
  {
    if ( auto element_access = dynamic_cast<ElementAccess*>( lhs.get() ) )
    {
      return std::make_unique<ElementAssignment>(
          location_for( *ctx ), consume, element_access->take_entity(),
          element_access->take_indexes(), std::move( rhs ) );
    }
    else if ( auto get_member = dynamic_cast<MemberAccess*>( lhs.get() ) )
    {
      return std::make_unique<MemberAssignment>( location_for( *ctx ), consume,
                                                 get_member->take_entity(), get_member->name,
                                                 std::move( rhs ), get_member->known_member );
    }
  }

  if ( token_id == TOK_ADDMEMBER || token_id == TOK_CHKMEMBER || token_id == TOK_DELMEMBER )
  {
    // On the right-hand side, any of the following are valid:
    //   - an identifier: treat as the field name
    //   - an expression: evaluate and use as the field name
    if ( auto identifier = dynamic_cast<Identifier*>( rhs.get() ) )
    {
      rhs = std::make_unique<StringValue>( rhs->source_location, identifier->name );
    }
  }

  auto op = std::make_unique<BinaryOperator>( location_for( *ctx ), std::move( lhs ),
                                              ctx->bop->getText(), token_id, std::move( rhs ) );
  if ( consume )
    return consume_expression_result( std::move( op ) );
  else
    return op;
}

BTokenId ExpressionBuilder::binary_operator_token(
    EscriptGrammar::EscriptParser::ExpressionContext* ctx )
{
  if ( ctx->ADD() )
    return TOK_ADD;
  else if ( ctx->SUB() )
    return TOK_SUBTRACT;
  else if ( ctx->MUL() )
    return TOK_MULT;
  else if ( ctx->DIV() )
    return TOK_DIV;
  else if ( ctx->ASSIGN() )
    return TOK_ASSIGN;
  else if ( ctx->EQUAL() )
    return TOK_EQUAL;
  else if ( ctx->NOTEQUAL_A() || ctx->NOTEQUAL_B() )
    return TOK_NEQ;
  else if ( ctx->LT() )
    return TOK_LESSTHAN;
  else if ( ctx->LE() )
    return TOK_LESSEQ;
  else if ( ctx->GT() )
    return TOK_GRTHAN;
  else if ( ctx->GE() )
    return TOK_GREQ;
  else if ( ctx->AND_A() || ctx->AND_B() )
    return TOK_AND;
  else if ( ctx->OR_A() || ctx->OR_B() )
    return TOK_OR;
  else if ( ctx->ADD_ASSIGN() )
    return TOK_PLUSEQUAL;
  else if ( ctx->SUB_ASSIGN() )
    return TOK_MINUSEQUAL;
  else if ( ctx->MUL_ASSIGN() )
    return TOK_TIMESEQUAL;
  else if ( ctx->DIV_ASSIGN() )
    return TOK_DIVIDEEQUAL;
  else if ( ctx->MOD() )
    return TOK_MODULUS;
  else if ( ctx->MOD_ASSIGN() )
    return TOK_MODULUSEQUAL;
  else if ( ctx->ADDMEMBER() )
    return TOK_ADDMEMBER;
  else if ( ctx->DELMEMBER() )
    return TOK_DELMEMBER;
  else if ( ctx->CHKMEMBER() )
    return TOK_CHKMEMBER;
  else if ( ctx->BITAND() )
    return TOK_BITAND;
  else if ( ctx->BITOR() )
    return TOK_BITOR;
  else if ( ctx->CARET() )
    return TOK_BITXOR;
  else if ( ctx->TOK_IN() )
    return TOK_IN;
  else if ( ctx->LSHIFT() )
    return TOK_BSLEFT;
  else if ( ctx->RSHIFT() )
    return TOK_BSRIGHT;
  else if ( ctx->ADDMEMBER() )
    return TOK_ADDMEMBER;
  else if ( ctx->CHKMEMBER() )
    return TOK_CHKMEMBER;
  else if ( ctx->DELMEMBER() )
    return TOK_DELMEMBER;
  else
    location_for( *ctx ).internal_error( "unrecognized binary operator" );
}

std::unique_ptr<DictionaryInitializer> ExpressionBuilder::dictionary_initializer(
    EscriptParser::ExplicitDictInitializerContext* ctx )
{
  std::vector<std::unique_ptr<DictionaryEntry>> entries;
  if ( auto initializer_ctx = ctx->dictInitializer() )
  {
    if ( auto list_ctx = initializer_ctx->dictInitializerExpressionList() )
    {
      for ( auto entry_ctx : list_ctx->dictInitializerExpression() )
      {
        auto loc = location_for( *entry_ctx );
        auto expressions = entry_ctx->expression();

        auto key = expression( expressions.at( 0 ) );
        auto value = ( expressions.size() >= 2 ) ? expression( expressions.at( 1 ) )
                                                 : std::make_unique<UninitializedValue>( loc );
        auto entry = std::make_unique<DictionaryEntry>( loc, std::move( key ), std::move( value ) );
        entries.push_back( std::move( entry ) );
      }
    }
  }
  auto loc = location_for( *ctx );

  return std::make_unique<DictionaryInitializer>( loc, std::move( entries ) );
}

std::unique_ptr<ElementAccess> ExpressionBuilder::element_access(
    std::unique_ptr<Expression> lhs, EscriptParser::ExpressionListContext* ctx )
{
  auto source_location = location_for( *ctx );

  std::vector<std::unique_ptr<Expression>> indexes;
  for ( auto expression_ctx : ctx->expression() )
  {
    indexes.push_back( expression( expression_ctx ) );
  }

  auto xx = std::make_unique<ElementIndexes>( location_for( *ctx ), std::move( indexes ) );

  return std::make_unique<ElementAccess>( source_location, std::move( lhs ), std::move( xx ) );
}

std::unique_ptr<ElvisOperator> ExpressionBuilder::elvis_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto source_location = location_for( *ctx );
  auto expressions = ctx->expression();
  auto lhs = expression( expressions[0] );
  auto rhs = expression( expressions[1] );
  return std::make_unique<ElvisOperator>( source_location, std::move( lhs ), std::move( rhs ) );
}

std::unique_ptr<ErrorInitializer> ExpressionBuilder::error(
    EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  auto source_location = location_for( *ctx );

  std::vector<std::string> identifiers;
  std::vector<std::unique_ptr<Expression>> expressions;

  if ( auto struct_initializer = ctx->structInitializer() )
  {
    if ( auto expression_list_ctx = struct_initializer->structInitializerExpressionList() )
    {
      for ( auto expression_ctx : expression_list_ctx->structInitializerExpression() )
      {
        auto expression_source_ctx = location_for( *expression_ctx );
        std::string identifier;
        if ( auto x = expression_ctx->IDENTIFIER() )
          identifier = text( x );
        else if ( auto string_literal = expression_ctx->STRING_LITERAL() )
          identifier = unquote( string_literal );
        else
          expression_source_ctx.internal_error(
              "Unable to determine identifier for struct initializer" );

        auto value = expression_ctx->expression() ? expression( expression_ctx->expression() )
                                                  : std::unique_ptr<Expression>();

        identifiers.push_back( std::move( identifier ) );
        expressions.push_back( std::move( value ) );
      }
    }
  }

  return std::make_unique<ErrorInitializer>( source_location, std::move( identifiers ),
                                             std::move( expressions ) );
}

std::vector<std::unique_ptr<Expression>> ExpressionBuilder::expressions(
    EscriptParser::ExpressionListContext* ctx )
{
  std::vector<std::unique_ptr<Expression>> expressions;
  if ( ctx )
  {
    for ( auto expression_ctx : ctx->expression() )
    {
      expressions.push_back( expression( expression_ctx ) );
    }
  }
  return expressions;
}

std::vector<std::unique_ptr<Expression>> ExpressionBuilder::expressions(
    EscriptParser::ArrayInitializerContext* ctx )
{
  if ( ctx )
  {
    if ( auto expression_list = ctx->expressionList() )
    {
      return expressions( expression_list );
    }
  }
  return {};
}

std::unique_ptr<Expression> ExpressionBuilder::expression( EscriptParser::ExpressionContext* ctx,
                                                           bool consume )
{
  std::unique_ptr<Expression> result;
  if ( auto prim = ctx->primary() )
    result = primary( prim );
  else if ( ctx->prefix )
    result = prefix_unary_operator( ctx );
  else if ( ctx->postfix )
    result = postfix_unary_operator( ctx );
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    if ( ctx->ELVIS() )
      result = elvis_operator( ctx );
    else
      return binary_operator( ctx, consume );
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    result = expression_suffix( expression( ctx->expression()[0] ), suffix );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled expression" );
  }

  if (consume)
  {
    result = consume_expression_result( std::move( result ) );
  }

  return result;
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

std::unique_ptr<MethodCall> ExpressionBuilder::method_call(
    std::unique_ptr<Expression> lhs, EscriptParser::MethodCallSuffixContext* ctx )
{
  auto loc = location_for( *ctx );
  auto methodname = text( ctx->IDENTIFIER() );
  auto arguments = expressions( ctx->expressionList() );

  auto argument_list = std::make_unique<MethodCallArgumentList>( loc, std::move( arguments ) );
  return std::make_unique<MethodCall>( loc, std::move( lhs ), std::move( methodname ),
                                       std::move( argument_list ) );
}

std::unique_ptr<MemberAccess> ExpressionBuilder::navigation(
    std::unique_ptr<Expression> lhs, EscriptGrammar::EscriptParser::NavigationSuffixContext* ctx )
{
  auto loc = location_for( *ctx );

  std::string name;
  if ( auto identifier = ctx->IDENTIFIER() )
    name = text( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    name = unquote( string_literal );
  else
    loc.internal_error( "member_access: need string literal or identifier" );
  return std::make_unique<MemberAccess>( loc, std::move( lhs ), std::move( name ) );
}

std::unique_ptr<Expression> ExpressionBuilder::expression_suffix(
    std::unique_ptr<Expression> lhs, EscriptParser::ExpressionSuffixContext* ctx )
{
  if ( auto indexing = ctx->indexingSuffix() )
  {
    return element_access( std::move( lhs ), indexing->expressionList() );
  }
  else if ( auto member = ctx->navigationSuffix() )
  {
    return navigation( std::move( lhs ), member );
  }
  else if ( auto method = ctx->methodCallSuffix() )
  {
    return method_call( std::move( lhs ), method );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled navigation suffix" );
  }
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto expression_ctx = ctx->expression( 0 );
  auto source_location = location_for( *expression_ctx );
  auto expression_ast = expression( expression_ctx );

  BTokenId token_id;
  if ( ctx->ADD() )
    token_id = TOK_UNPLUS;
  else if ( ctx->SUB() )
    token_id = TOK_UNMINUS;
  else if ( ctx->INC() )
    token_id = TOK_UNPLUSPLUS;
  else if ( ctx->DEC() )
    token_id = TOK_UNMINUSMINUS;
  else if ( ctx->TILDE() )
    token_id = TOK_BITWISE_NOT;
  else if ( ctx->BANG_A() || ctx->BANG_B() )
    token_id = TOK_LOG_NOT;
  else
    unhandled_operator( source_location );

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

  BTokenId token_id;
  if ( ctx->INC() )
    token_id = TOK_UNPLUSPLUS_POST;
  else if ( ctx->DEC() )
    token_id = TOK_UNMINUSMINUS_POST;
  else
    unhandled_operator( loc );

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
  else if ( auto dict_init = ctx->explicitDictInitializer() )
  {
    return dictionary_initializer( dict_init );
  }
  else if ( auto struct_init = ctx->explicitStructInitializer() )
  {
    return struct_initializer( struct_init );
  }
  else if ( auto fr = ctx->functionReference() )
  {
    return function_reference( fr );
  }
  else if ( auto error_init = ctx->explicitErrorInitializer() )
  {
    return error( error_init );
  }
  else if ( auto array_init = ctx->explicitArrayInitializer() )
  {
    return array_initializer( array_init );
  }
  else if ( auto bare_array = ctx->bareArrayInitializer() )
  {
    return array_initializer( bare_array );
  }

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::ScopedFunctionCallContext* ctx )
{
  return function_call( ctx->functionCall(), text( ctx->IDENTIFIER() ) );
}

std::unique_ptr<Expression> ExpressionBuilder::struct_initializer(
    EscriptParser::ExplicitStructInitializerContext* ctx )
{
  std::vector<std::unique_ptr<StructMemberInitializer>> initializers;

  if ( auto struct_init = ctx->structInitializer() )
  {
    if ( auto expression_list_ctx = struct_init->structInitializerExpressionList() )
    {
      for ( auto initializer_expression_ctx : expression_list_ctx->structInitializerExpression() )
      {
        auto loc = location_for( *initializer_expression_ctx );
        std::string identifier;
        if ( auto x = initializer_expression_ctx->IDENTIFIER() )
          identifier = text( x );
        else if ( auto string_literal = initializer_expression_ctx->STRING_LITERAL() )
          identifier = unquote( string_literal );
        else
          loc.internal_error( "Unable to determine identifier for struct initializer" );

        if ( auto expression_ctx = initializer_expression_ctx->expression() )
        {
          auto initializer = expression( expression_ctx );
          initializers.push_back( std::make_unique<StructMemberInitializer>(
              loc, std::move( identifier ), std::move( initializer ) ) );
        }
        else
        {
          initializers.push_back(
              std::make_unique<StructMemberInitializer>( loc, std::move( identifier ) ) );
        }
      }
    }
  }
  return std::make_unique<StructInitializer>( location_for( *ctx ), std::move( initializers ) );
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
