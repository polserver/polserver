#include "ExpressionBuilder.h"

#include "compiler/Report.h"
#include <boost/range/adaptor/reversed.hpp>

#include "BuilderWorkspace.h"
#include "compiler/Report.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/ArrayElementAccess.h"
#include "compiler/ast/ArrayElementIndexes.h"
#include "compiler/ast/ArrayInitializer.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/DictionaryEntry.h"
#include "compiler/ast/DictionaryInitializer.h"
#include "compiler/ast/ElvisOperator.h"
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
#include "compiler/ast/Value.h"

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

std::unique_ptr<Expression> ExpressionBuilder::additive_expression(
    EscriptParser::AdditiveExpressionContext* ctx )
{
  auto operands = ctx->multiplicativeExpression();

  if ( operands.size() == 1 )
    return multiplicative_expression( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->additiveOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    auto ast = multiplicative_expression( operand_ctx );
    if ( !result )
    {
      result = std::move( ast );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      std::string op_string = op->getText();
      BTokenId token_id =
          op->ADD() ? TOK_ADD : op->SUB() ? TOK_SUBTRACT : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, std::move( ast ) );
    }
  }

  return result;
}

std::unique_ptr<ArrayElementAccess> ExpressionBuilder::array_access(
    EscriptParser::ExpressionContext* ctx )
{
  auto source_location = location_for( *ctx );
  auto lhs = expression( ctx->expression()[0] );

  std::vector<std::unique_ptr<Expression>> indexes;
  auto expression_list = ctx->expressionList();
  if ( expression_list )
  {
    for ( auto expression_ctx : expression_list->expression() )
    {
      indexes.push_back( expression( expression_ctx ) );
    }
  }

  auto xx = std::make_unique<ArrayElementIndexes>( location_for( *expression_list ),
                                                     std::move( indexes ) );

  return std::make_unique<ArrayElementAccess>( source_location, std::move( lhs ),
                                                 std::move( xx ) );
}

std::unique_ptr<ArrayElementAccess> ExpressionBuilder::array_access(
    std::unique_ptr<Expression> lhs, EscriptParser::UnambiguousExpressionListContext* ctx )
{
  auto loc = location_for( *ctx );

  std::vector<std::unique_ptr<Expression>> indexes;
  for ( auto expression_ctx : ctx->unambiguousExpression() )
  {
    indexes.push_back( expression( expression_ctx ) );
  }

  return std::make_unique<ArrayElementAccess>(
      loc, std::move( lhs ), std::make_unique<ArrayElementIndexes>( loc, std::move( indexes ) ) );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::ExpressionContext* ctx )
{
  auto values = expressions( ctx->expressionList() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::ArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->expressionList() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::UnambiguousBareArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->unambiguousExpressionList() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<ArrayInitializer> ExpressionBuilder::array_initializer(
    EscriptParser::UnambiguousExplicitArrayInitializerContext* ctx )
{
  auto values = expressions( ctx->unambiguousArrayInitializer() );
  return std::make_unique<ArrayInitializer>( location_for( *ctx ), std::move( values ) );
}

std::unique_ptr<Expression> ExpressionBuilder::atomic( EscriptParser::AtomicExpressionContext* ctx )
{
  if ( auto id = ctx->IDENTIFIER() )
  {
    return std::make_unique<Identifier>( location_for( *ctx ), text( id ) );
  }
  else if ( auto literal = ctx->literal() )
  {
    return value( literal );
  }
  else if ( auto fcall = ctx->unambiguousFunctionCall() )
  {
    return function_call( fcall, "" );
  }
  else if ( auto parenthesized = ctx->parenthesizedExpression() )
  {
    return expression( parenthesized->unambiguousExpression() );
  }
  else if ( auto bare_array_initializer = ctx->unambiguousBareArrayInitializer() )
  {
    return array_initializer( bare_array_initializer );
  }
  else if ( auto dict_init = ctx->unambiguousExplicitDictInitializer() )
  {
    return dictionary_initializer( dict_init );
  }
  else if ( auto struct_init = ctx->unambiguousExplicitStructInitializer() )
  {
    return struct_initializer( struct_init );
  }
  else if ( auto error_init = ctx->unambiguousExplicitErrorInitializer() )
  {
    return error( error_init );
  }
  else if ( auto array_init = ctx->unambiguousExplicitArrayInitializer() )
  {
    return array_initializer( array_init );
  }
  else if ( auto funcref_init = ctx->functionReference() )
  {
    return function_reference( funcref_init );
  }
  else if ( auto scoped = ctx->unambiguousScopedFunctionCall() )
  {
    return scoped_function_call( scoped );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled atomic" );
  }
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
  else if ( ctx->EQUAL() || ctx->EQUAL_DEPRECATED() )
  {
    if ( ctx->EQUAL_DEPRECATED() )
      report.error( location_for( *ctx ), "Deprecated '=' found: did you mean '==' or ':='?\n" );
    token_id = TOK_EQUAL;
  }
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

std::unique_ptr<Expression> ExpressionBuilder::bitshift_left(
    EscriptParser::BitshiftLeftContext* ctx )
{
  auto operands = ctx->comparison();

  if ( operands.size() == 1 )
    return comparison( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->LSHIFT();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = comparison( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = TOK_BSLEFT;
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, comparison( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::bitshift_right(
    EscriptParser::BitshiftRightContext* ctx )
{
  auto operands = ctx->bitshiftLeft();

  if ( operands.size() == 1 )
    return bitshift_left( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->RSHIFT();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = bitshift_left( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = TOK_BSRIGHT;
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, bitshift_left( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_conjunction(
    EscriptParser::BitwiseConjunctionContext* ctx )
{
  auto operands = ctx->bitshiftRight();

  if ( operands.size() == 1 )
    return bitshift_right( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->BITAND();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = bitshift_right( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = TOK_BITAND;
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, bitshift_right( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_disjunction(
    EscriptParser::BitwiseDisjunctionContext* ctx )
{
  auto operands = ctx->bitwiseXor();

  if ( operands.size() == 1 )
    return bitwise_xor( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->BITOR();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = bitwise_xor( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = TOK_BITOR;
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, bitwise_xor( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::bitwise_xor( EscriptParser::BitwiseXorContext* ctx )
{
  auto operands = ctx->bitwiseConjunction();

  if ( operands.size() == 1 )
    return bitwise_conjunction( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->CARET();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    auto ast = bitwise_conjunction( operand_ctx );
    if ( !result )
    {
      result = std::move( ast );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = TOK_BITXOR;
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, std::move( ast ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::comparison( EscriptParser::ComparisonContext* ctx )
{
  auto operands = ctx->infixOperation();

  if ( operands.size() == 1 )
    return infix_operation( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->comparisonOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = infix_operation( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id =
          op->LT()
              ? TOK_LESSTHAN
              : op->LE() ? TOK_LESSEQ
                         : op->GT() ? TOK_GRTHAN : op->GE() ? TOK_GREQ : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, infix_operation( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::conjunction( EscriptParser::ConjunctionContext* ctx )
{
  auto operands = ctx->equality();

  if ( operands.size() == 1 )
    return equality( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->conjunctionOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = equality( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = ( op->AND_A() || op->AND_B() ) ? TOK_AND : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, equality( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<DictionaryInitializer> ExpressionBuilder::dictionary_initializer(
    EscriptParser::ExpressionContext* ctx )
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
        auto entry =
            std::make_unique<DictionaryEntry>( loc, std::move( key ), std::move( value ) );
        entries.push_back( std::move( entry ) );
      }
    }
  }
  auto loc = location_for( *ctx );

  return std::make_unique<DictionaryInitializer>( loc, std::move( entries ) );
}

std::unique_ptr<DictionaryInitializer> ExpressionBuilder::dictionary_initializer(
    EscriptParser::UnambiguousExplicitDictInitializerContext* ctx )
{
  std::vector<std::unique_ptr<DictionaryEntry>> entries;
  if ( auto initializer_ctx = ctx->unambiguousDictInitializer() )
  {
    if ( auto list_ctx = initializer_ctx->unambiguousDictInitializerExpressionList() )
    {
      for ( auto entry_ctx : list_ctx->unambiguousDictInitializerExpression() )
      {
        auto source_location = location_for( *entry_ctx );
        auto expressions = entry_ctx->unambiguousExpression();

        auto key = expression( expressions.at( 0 ) );
        auto value = ( expressions.size() >= 2 )
                         ? expression( expressions.at( 1 ) )
                         : std::make_unique<UninitializedValue>( source_location );
        auto entry = std::make_unique<DictionaryEntry>( source_location, std::move( key ),
                                                          std::move( value ) );
        entries.push_back( std::move( entry ) );
      }
    }
  }
  auto source_location = location_for( *ctx );

  return std::make_unique<DictionaryInitializer>( source_location, std::move( entries ) );
}

std::unique_ptr<Expression> ExpressionBuilder::disjunction( EscriptParser::DisjunctionContext* ctx )
{
  auto operands = ctx->conjunction();

  if ( operands.size() == 1 )
    return conjunction( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->disjunctionOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = conjunction( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      BTokenId token_id = TOK_OR;
      result =
          std::make_unique<BinaryOperator>( location_for( *op ), std::move( result ),
                                              op->getText(), token_id, conjunction( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::elvis_expression(
    EscriptParser::ElvisExpressionContext* ctx )
{
  auto operands = ctx->additiveExpression();

  if ( operands.size() == 1 )
    return additive_expression( operands[0] );

  auto operators = ctx->ELVIS();
  std::unique_ptr<Expression> result;
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = additive_expression( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      result = std::make_unique<ElvisOperator>( location_for( *op ), std::move( result ),
                                                  additive_expression( operand_ctx ) );
    }
  }

  return result;
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

std::unique_ptr<Expression> ExpressionBuilder::equality( EscriptParser::EqualityContext* ctx )
{
  auto operands = ctx->bitwiseDisjunction();

  if ( operands.size() == 1 )
    return bitwise_disjunction( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->equalityOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = bitwise_disjunction( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );

      if ( op->EQUAL_DEPRECATED() )
        report.error( location_for( *ctx ), "Deprecated '=' found: did you mean '==' or ':='?\n" );

      BTokenId token_id =
          ( op->EQUAL() || op->EQUAL_DEPRECATED() )
              ? TOK_EQUAL
              : ( op->NOTEQUAL_A() || op->NOTEQUAL_B() ) ? TOK_NEQ : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, bitwise_disjunction( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<ErrorInitializer> ExpressionBuilder::error( EscriptParser::ExpressionContext* ctx )
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

std::unique_ptr<ErrorInitializer> ExpressionBuilder::error(
    EscriptParser::UnambiguousExplicitErrorInitializerContext* ctx )
{
  auto source_location = location_for( *ctx );

  std::vector<std::string> identifiers;
  std::vector<std::unique_ptr<Expression>> expressions;

  if ( auto struct_initializer = ctx->unambiguousStructInitializer() )
  {
    if ( auto expression_list_ctx =
             struct_initializer->unambiguousStructInitializerExpressionList() )
    {
      for ( auto expression_ctx : expression_list_ctx->unambiguousStructInitializerExpression() )
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

        auto value = expression_ctx->unambiguousExpression()
                         ? expression( expression_ctx->unambiguousExpression() )
                         : std::unique_ptr<Expression>();

        identifiers.push_back( std::move( identifier ) );
        expressions.push_back( std::move( value ) );
      }
    }
  }

  return std::make_unique<ErrorInitializer>( source_location, identifiers,
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
    EscriptParser::UnambiguousExpressionListContext* ctx )
{
  std::vector<std::unique_ptr<Expression>> expressions;
  if ( ctx )
  {
    for ( auto expression_ctx : ctx->unambiguousExpression() )
    {
      expressions.push_back( expression( expression_ctx ) );
    }
  }
  return expressions;
}

std::vector<std::unique_ptr<Expression>> ExpressionBuilder::expressions(
    EscriptParser::UnambiguousArrayInitializerContext* ctx )
{
  if ( ctx )
  {
    if ( auto expression_list = ctx->unambiguousExpressionList() )
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
  else if ( auto m_call = ctx->methodCall() )
    return function_call( m_call, "" );
  else if ( auto scoped_method_call = ctx->scopedMethodCall() )
    return scoped_function_call( scoped_method_call );
  else if ( ctx->prefix )
    return prefix_unary_operator( ctx );
  else if ( ctx->postfix )
    return postfix_unary_operator( ctx );
  else if ( ctx->TOK_ERROR() )
    return error( ctx );
  else if ( auto s_init = ctx->structInitializer() )
    return struct_initializer( s_init );
  else if ( ctx->STRUCT() )
  {
    std::vector<std::unique_ptr<StructMemberInitializer>> no_initializers;
    return std::make_unique<StructInitializer>( location_for( *ctx ),
                                                  std::move( no_initializers ) );
  }
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    if ( ctx->ELVIS() )
      return elvis_operator( ctx );
    else
      return binary_operator( ctx );
  }
  else if ( ctx->bop && ctx->expression().size() == 1 &&
            ( ctx->IDENTIFIER() || ctx->STRING_LITERAL() ) &&
            ( ctx->ADDMEMBER() || ctx->DELMEMBER() || ctx->CHKMEMBER() ) )
    return membership_operator( ctx );
  else if ( ctx->ARRAY() && ctx->arrayInitializer() )
    return array_initializer( ctx->arrayInitializer() );
  else if ( ( ctx->ARRAY() && !ctx->arrayInitializer() ) || ( ctx->LBRACE() && ctx->RBRACE() ) )
    return array_initializer( ctx );
  else if ( ctx->bop && ctx->DOT() && ctx->expression().size() == 1 &&
            ( ctx->IDENTIFIER() || ctx->STRING_LITERAL() ) )
    return member_access( ctx );
  else if ( ctx->bop && ctx->DOT() && ctx->expression().size() == 1 && ctx->memberCall() )
    return method_call( ctx );
  else if ( ctx->expression().size() == 1 && ctx->LBRACK() && ctx->RBRACK() &&
            ctx->expressionList() )
    return array_access( ctx );
  else if ( ctx->DICTIONARY() )
    return dictionary_initializer( ctx );
  else if ( auto fr = ctx->functionReference() )
    return function_reference( fr );

  location_for( *ctx ).internal_error( "unhandled expression" );
}

std::unique_ptr<Expression> ExpressionBuilder::expression(
    EscriptParser::UnambiguousExpressionContext* ctx )
{
  auto operands = ctx->membership();
  if ( operands.size() == 1 )
    return membership( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->assignmentOperator();
  auto operators_itr = operators.rbegin();
  for ( auto operand_ctx : boost::adaptors::reverse( operands ) )
  {
    if ( !result )
    {
      result = membership( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = op->ASSIGN() ? TOK_ASSIGN
                                       : op->ADD_ASSIGN()
                                             ? TOK_PLUSEQUAL
                                             : op->SUB_ASSIGN()
                                                   ? TOK_MINUSEQUAL
                                                   : op->MUL_ASSIGN()
                                                         ? TOK_TIMESEQUAL
                                                         : op->DIV_ASSIGN()
                                                               ? TOK_DIVIDEEQUAL
                                                               : op->MOD_ASSIGN()
                                                                     ? TOK_MODULUSEQUAL
                                                                     : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, membership( operand_ctx ), op->getText(),
                                                   token_id, std::move( result ) );
    }
  }

  return result;
}

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::MethodCallContext* ctx, const std::string& scope )
{
  auto method_name = text( ctx->IDENTIFIER() );

  std::vector<std::unique_ptr<Argument>> arguments;

  if ( auto argList = ctx->methodCallArgumentList() )
  {
    for ( auto argument_context : argList->methodCallArgument() )
    {
      std::string identifier =
          argument_context->IDENTIFIER() ? text( argument_context->IDENTIFIER() ) : "";
      auto value = expression( argument_context->expression() );
      auto argument = std::make_unique<Argument>( location_for( *argument_context ),
                                                    std::move( identifier ), std::move( value ) );
      arguments.push_back( std::move( argument ) );
    }
  }
  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, method_name,
                                                         std::move( arguments ) );

  std::string key = scope.empty() ? method_name : ( scope + "::" + method_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
}

std::unique_ptr<FunctionCall> ExpressionBuilder::function_call(
    EscriptParser::UnambiguousFunctionCallContext* ctx, const std::string& scope )
{
  auto function_name = text( ctx->IDENTIFIER() );
  auto arguments = value_arguments( ctx->valueArguments() );
  auto function_call = std::make_unique<FunctionCall>( location_for( *ctx ), scope, function_name,
                                                         std::move( arguments ) );

  std::string key = scope.empty() ? function_name : ( scope + "::" + function_name );
  workspace.function_resolver.register_function_link( key, function_call->function_link );

  return function_call;
}

std::unique_ptr<Expression> ExpressionBuilder::infix_operation(
    EscriptParser::InfixOperationContext* ctx )
{
  auto operands = ctx->elvisExpression();

  if ( operands.size() == 1 )
    return elvis_expression( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->infixOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = elvis_expression( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = op->TOK_IN() ? TOK_IN : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, elvis_expression( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<GetMember> ExpressionBuilder::member_access( EscriptParser::ExpressionContext* ctx )
{
  auto loc = location_for( *ctx );
  auto entity = expression( ctx->expression()[0] );
  std::string name;
  if ( auto identifier = ctx->IDENTIFIER() )
    name = text( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    name = unquote( string_literal );
  else
    loc.internal_error( "member_access: need string literal or identifier" );
  return std::make_unique<GetMember>( loc, std::move( entity ), std::move( name ) );
}

std::unique_ptr<MethodCall> ExpressionBuilder::method_call( EscriptParser::ExpressionContext* ctx )
{
  auto loc = location_for( *ctx );
  auto entity = expression( ctx->expression()[0] );
  auto memberCall = ctx->memberCall();
  auto methodname = text( memberCall->IDENTIFIER() );
  auto expression_lst = memberCall->expressionList();

  std::vector<std::unique_ptr<Expression>> arguments;
  if ( expression_lst )
  {
    for ( auto expression_ctx : expression_lst->expression() )
    {
      arguments.push_back( expression( expression_ctx ) );
    }
  }

  auto argument_list = std::make_unique<MethodCallArgumentList>( loc, std::move( arguments ) );
  return std::make_unique<MethodCall>( loc, std::move( entity ), std::move( methodname ),
                                         std::move( argument_list ) );
}

std::unique_ptr<MethodCall> ExpressionBuilder::method_call(
    std::unique_ptr<Expression> lhs, EscriptParser::MemberCallSuffixContext* ctx )
{
  auto loc = location_for( *ctx );
  auto methodname = text( ctx->IDENTIFIER() );

  auto arguments = expressions( ctx->unambiguousExpressionList() );
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
    rhs = std::make_unique<StringValue>( loc, identifier->name );
  }

  return std::make_unique<BinaryOperator>( loc, std::move( lhs ), std::move( op ), token_id,
                                             std::move( rhs ) );
}

std::unique_ptr<Expression> ExpressionBuilder::membership( EscriptParser::MembershipContext* ctx )
{
  auto operands = ctx->disjunction();
  if ( operands.size() == 1 )
    return disjunction( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->membershipOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    auto ast = disjunction( operand_ctx );
    if ( !result )
    {
      result = std::move( ast );
    }
    else
    {
      if ( auto identifier = dynamic_cast<Identifier*>( ast.get() ) )
      {
        // these can be either of these forms, which are equivalent:
        //  a.+b := 42;
        //  or
        //  a.+"b" := 42;
        ast = std::make_unique<StringValue>( identifier->source_location, identifier->name );
      }
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id = op->ADDMEMBER()
                              ? TOK_ADDMEMBER
                              : op->DELMEMBER()
                                    ? TOK_DELMEMBER
                                    : op->CHKMEMBER() ? TOK_CHKMEMBER : unhandled_operator( loc );
      result = std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(),
                                                   token_id, std::move( ast ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::multiplicative_expression(
    EscriptParser::MultiplicativeExpressionContext* ctx )
{
  auto operands = ctx->prefixUnaryInversionExpression();

  if ( operands.size() == 1 )
    return prefix_unary_inversion( operands[0] );

  std::unique_ptr<Expression> result;
  auto operators = ctx->multiplicativeOperator();
  auto operators_itr = operators.begin();
  for ( auto operand_ctx : operands )
  {
    if ( !result )
    {
      result = prefix_unary_inversion( operand_ctx );
    }
    else
    {
      auto op = *operators_itr++;
      auto loc = location_for( *op );
      BTokenId token_id =
          op->MUL() ? TOK_MULT
                    : op->DIV() ? TOK_DIV : op->MOD() ? TOK_MODULUS : unhandled_operator( loc );
      result =
          std::make_unique<BinaryOperator>( loc, std::move( result ), op->getText(), token_id,
                                              prefix_unary_inversion( operand_ctx ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::navigation_suffix(
    std::unique_ptr<Expression> lhs, EscriptParser::NavigationSuffixContext* ctx )
{
  auto loc = location_for( *ctx );

  std::string name;
  if ( auto identifier = ctx->IDENTIFIER() )
    name = text( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    name = unquote( string_literal );
  else
    loc.internal_error( "expected an identifier or a string literal" );
  return std::make_unique<GetMember>( loc, std::move( lhs ), std::move( name ) );
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_arithmetic(
    EscriptParser::PrefixUnaryArithmeticExpressionContext* ctx )
{
  auto result = postfix_unary( ctx->postfixUnaryExpression() );

  auto operators = ctx->prefixUnaryArithmeticOperator();

  for ( auto op : boost::adaptors::reverse( operators ) )
  {
    auto loc = location_for( *op );
    BTokenId token_id = op->SUB() ? TOK_UNMINUS
                                  : op->INC() ? TOK_UNPLUSPLUS
                                              : op->ADD() ? TOK_UNPLUS
                                                          : op->DEC() ? TOK_UNMINUSMINUS
                                                                      : unhandled_operator( loc );

    if ( token_id != TOK_UNPLUS )
    {
      result =
          std::make_unique<UnaryOperator>( loc, op->getText(), token_id, std::move( result ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::prefix_unary_inversion(
    EscriptParser::PrefixUnaryInversionExpressionContext* ctx )
{
  auto result = prefix_unary_arithmetic( ctx->prefixUnaryArithmeticExpression() );

  auto operators = ctx->prefixUnaryInversionOperator();

  for ( auto op : boost::adaptors::reverse( operators ) )
  {
    auto loc = location_for( *op );
    BTokenId token_id = ( op->BANG_A() || op->BANG_B() )
                            ? TOK_LOG_NOT
                            : op->TILDE() ? TOK_BITWISE_NOT : unhandled_operator( loc );
    result = std::make_unique<UnaryOperator>( loc, op->getText(), token_id, std::move( result ) );
  }

  return result;
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

std::unique_ptr<Expression> ExpressionBuilder::postfix_unary(
    EscriptParser::PostfixUnaryExpressionContext* ctx )
{
  auto result = atomic( ctx->atomicExpression() );

  auto operators = ctx->postfixUnaryOperator();

  for ( auto& op : operators )
  {
    auto loc = location_for( *op );
    if ( auto indexing_suffix = op->indexingSuffix() )
    {
      result = array_access( std::move( result ), indexing_suffix->unambiguousExpressionList() );
    }
    else if ( auto call_suffix = op->memberCallSuffix() )
    {
      result = method_call( std::move( result ), call_suffix );
    }
    else if ( auto navigation_suffix_ctx = op->navigationSuffix() )
    {
      result = navigation_suffix( std::move( result ), navigation_suffix_ctx );
    }
    else
    {
      BTokenId token_id = op->INC() ? TOK_UNPLUSPLUS_POST
                                    : op->DEC() ? TOK_UNMINUSMINUS_POST : unhandled_operator( loc );
      result =
          std::make_unique<UnaryOperator>( loc, op->getText(), token_id, std::move( result ) );
    }
  }

  return result;
}

std::unique_ptr<Expression> ExpressionBuilder::primary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
    return value( literal );
  else if ( auto prim_expr_ctx = ctx->expression() )
    return expression( prim_expr_ctx );
  else if ( auto identifier = ctx->IDENTIFIER() )
    return std::make_unique<Identifier>( location_for( *ctx ), text( identifier ) );

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::ScopedMethodCallContext* ctx )
{
  return function_call( ctx->methodCall(), text( ctx->IDENTIFIER() ) );
}

std::unique_ptr<FunctionCall> ExpressionBuilder::scoped_function_call(
    EscriptParser::UnambiguousScopedFunctionCallContext* ctx )
{
  return function_call( ctx->unambiguousFunctionCall(), text( ctx->IDENTIFIER() ) );
}


std::unique_ptr<Expression> ExpressionBuilder::struct_initializer(
    EscriptParser::StructInitializerContext* ctx )
{
  std::vector<std::unique_ptr<StructMemberInitializer>> initializers;

  if ( auto expression_list_ctx = ctx->structInitializerExpressionList() )
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
  return std::make_unique<StructInitializer>( location_for( *ctx ), std::move( initializers ) );
}

std::unique_ptr<Expression> ExpressionBuilder::struct_initializer(
    EscriptParser::UnambiguousExplicitStructInitializerContext* ctx )
{
  std::vector<std::unique_ptr<StructMemberInitializer>> initializers;

  if ( auto struct_init = ctx->unambiguousStructInitializer() )
  {
    if ( auto expression_list_ctx = struct_init->unambiguousStructInitializerExpressionList() )
    {
      for ( auto expression_ctx : expression_list_ctx->unambiguousStructInitializerExpression() )
      {
        auto loc = location_for( *expression_ctx );
        std::string identifier;
        if ( auto x = expression_ctx->IDENTIFIER() )
        {
          identifier = text( x );
        }
        else if ( auto string_literal = expression_ctx->STRING_LITERAL() )
        {
          identifier = unquote( string_literal );
        }
        else
        {
          loc.internal_error( "Unable to determine identifier for struct initializer." );
        }

        if ( auto initializer_ctx = expression_ctx->unambiguousExpression() )
        {
          initializers.push_back( std::make_unique<StructMemberInitializer>(
              loc, std::move( identifier ), expression( initializer_ctx ) ) );
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
    EscriptParser::ValueArgumentsContext* ctx )
{
  std::vector<std::unique_ptr<Argument>> arguments;

  if ( ctx )
  {
    if ( auto argList = ctx->unambiguousFunctionCallArgumentList() )
    {
      for ( auto argument_context : argList->unambiguousFunctionCallArgument() )
      {
        auto loc = location_for( *argument_context );

        std::string name;
        auto value = expression( argument_context->unambiguousExpression() );

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
  }
  return arguments;
}

}  // namespace Pol::Bscript::Compiler
