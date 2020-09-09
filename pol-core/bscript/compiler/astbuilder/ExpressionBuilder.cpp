#include "ExpressionBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/ArrayInitializer.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/DictionaryEntry.h"
#include "compiler/ast/DictionaryInitializer.h"
#include "compiler/ast/ElementAccess.h"
#include "compiler/ast/ElementIndexes.h"
#include "compiler/ast/ErrorInitializer.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/FunctionReference.h"
#include "compiler/ast/GetMember.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/MethodCall.h"
#include "compiler/ast/MethodCallArgumentList.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/StructInitializer.h"
#include "compiler/ast/StructMemberInitializer.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/UninitializedValue.h"
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

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::ArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->expressionList() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
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
    if ( ctx->ADDMEMBER() || ctx->DELMEMBER() || ctx->CHKMEMBER() )
      return membership_operator( ctx );
    else
      return binary_operator( ctx );
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    return expression_suffix( expression( ctx->expression()[0] ), suffix );
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

std::unique_ptr<MethodCall> ExpressionBuilder::method_call(
    std::unique_ptr<Expression> lhs, EscriptParser::MethodCallSuffixContext* ctx )
{
  auto loc = location_for( *ctx );
  auto methodname = text( ctx->IDENTIFIER() );
  auto expression_lst = ctx->expressionList();

  std::vector<std::unique_ptr<Expression>> arguments;
  if ( expression_lst )
  {
    for ( auto expression_ctx : expression_lst->expression() )
    {
      arguments.push_back( expression( expression_ctx ) );
    }
  }

  auto argument_list = std::make_unique<MethodCallArgumentList>( loc, std::move( arguments ) );
  return std::make_unique<MethodCall>( loc, std::move( lhs ), std::move( methodname ),
                                       std::move( argument_list ) );
}

std::unique_ptr<BinaryOperator> ExpressionBuilder::membership_operator(
    EscriptParser::ExpressionContext* ctx )
{
  auto loc = location_for( *ctx );
  std::unique_ptr<Expression> lhs = expression( ctx->expression( 0 ) );
  std::string op = ctx->bop->getText();

  BTokenId token_id = ctx->ADDMEMBER()
                          ? TOK_ADDMEMBER
                          : ctx->DELMEMBER()
                                ? TOK_DELMEMBER
                                : ctx->CHKMEMBER() ? TOK_CHKMEMBER : unhandled_operator( loc );

  // On the right-hand side, any of the following are valid:
  //   - an identifier: treat as the field name
  //   - an expression: evaluate and use as the field name
  std::unique_ptr<Expression> rhs = expression( ctx->expression( 1 ) );
  if ( auto identifier = dynamic_cast<Identifier*>( rhs.get() ) )
  {
    rhs = std::make_unique<StringValue>( rhs->source_location, identifier->name );
  }

  return std::make_unique<BinaryOperator>( loc, std::move( lhs ), std::move( op ), token_id,
                                           std::move( rhs ) );
}

std::unique_ptr<GetMember> ExpressionBuilder::navigation(
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
  return std::make_unique<GetMember>( loc, std::move( lhs ), std::move( name ) );
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
