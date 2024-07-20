#include "ValueBuilder.h"

#include <cstring>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "clib/strutil.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ValueBuilder::ValueBuilder( const SourceFileIdentifier& source_file_identifier,
                            BuilderWorkspace& workspace )
    : TreeBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<BooleanValue> ValueBuilder::bool_value(
    EscriptGrammar::EscriptParser::BoolLiteralContext* ctx )
{
  bool value;
  auto loc = location_for( *ctx );

  if ( ctx->BOOL_TRUE() )
  {
    value = true;
  }
  else if ( ctx->BOOL_FALSE() )
  {
    value = false;
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled boolean literal" );
  }

  return std::make_unique<BooleanValue>( loc, value );
}

std::unique_ptr<FloatValue> ValueBuilder::float_value(
    EscriptGrammar::EscriptParser::FloatLiteralContext* ctx )
{
  auto loc = location_for( *ctx );
  auto terminal = ctx->FLOAT_LITERAL();
  if ( !terminal )
  {
    terminal = ctx->HEX_FLOAT_LITERAL();
    if ( !terminal )
    {
      location_for( *ctx ).internal_error( "unhandled float literal" );
    }
  }
  double value = std::stod( text( terminal ) );
  return std::make_unique<FloatValue>( loc, value );
}

std::unique_ptr<FunctionReference> ValueBuilder::function_reference(
    EscriptParser::FunctionReferenceContext* ctx )
{
  auto source_location = location_for( *ctx );
  auto name = ctx->IDENTIFIER()->getSymbol()->getText();

  auto function_link = std::make_shared<FunctionLink>( source_location );
  auto function_reference =
      std::make_unique<FunctionReference>( source_location, name, function_link );

  workspace.function_resolver.register_function_link( name, function_link );

  return function_reference;
}

std::unique_ptr<BooleanValue> ValueBuilder::function_expression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto loc = location_for( *ctx );
  return std::make_unique<BooleanValue>( loc, true );
}

std::unique_ptr<IntegerValue> ValueBuilder::integer_value(
    EscriptParser::IntegerLiteralContext* ctx )
{
  auto loc = location_for( *ctx );
  return std::make_unique<IntegerValue>( loc, to_int( ctx ) );
}

std::unique_ptr<StringValue> ValueBuilder::string_value( antlr4::tree::TerminalNode* string_literal,
                                                         bool expect_quotes )
{
  auto loc = location_for( *string_literal );
  return std::make_unique<StringValue>( loc, unquote( string_literal, expect_quotes ) );
}

std::string ValueBuilder::unquote( antlr4::tree::TerminalNode* string_literal, bool expect_quotes )
{
  std::string input = string_literal->getSymbol()->getText();
  const char* s = input.c_str();
  if ( *s != '\"' && expect_quotes )
    location_for( *string_literal ).internal_error( "string does not begin with a quote?" );

  const char* end = s + ( expect_quotes ? 1 : 0 );
  std::string lit;
  lit.reserve( input.length() );
  bool escnext = false;  // true when waiting for 2nd char in an escape sequence
  u8 hexnext = 0;        // tells how many more chars in a \xNN escape sequence
  char hexstr[3];        // will contain the \x escape chars to be processed
  memset( hexstr, 0, 3 );

  for ( ;; )
  {
    if ( !*end )
    {
      if ( !expect_quotes )
        break;
      // parser should catch this.
      location_for( *string_literal ).internal_error( "unterminated string" );
    }

    if ( escnext && hexnext )
      location_for( *string_literal )
          .internal_error( "Bug in the compiler. Please report this on the forums." );

    if ( escnext )
    {
      // waiting for 2nd character after a backslash
      escnext = false;
      if ( *end == 'n' )
        lit += '\n';
      else if ( *end == 't' )
        lit += '\t';
      else if ( *end == 'x' )
        hexnext = 2;
      else
        lit += *end;
    }
    else if ( hexnext )
    {
      // waiting for next (two) chars in hex escape sequence (eg. \xFF)
      hexstr[2 - hexnext] = *end;
      if ( !--hexnext )
      {
        char* endptr;
        char ord = static_cast<char>( strtol( hexstr, &endptr, 16 ) );
        if ( *endptr != '\0' )
        {
          report.error( location_for( *string_literal ), "Invalid hex escape sequence '{}'.",
                        hexstr );
          return lit;
        }
        lit += ord;
      }
    }
    else
    {
      if ( *end == '\\' )
        escnext = true;
      else if ( *end == '\"' )
        break;
      else
        lit += *end;
    }
    ++end;
  }
  if ( !Clib::isValidUnicode( lit ) )
  {
    report.warning( location_for( *string_literal ),
                    "Warning: invalid unicode character detected. Assuming ISO8859." );

    Clib::sanitizeUnicodeWithIso( &lit );
  }
  return lit;
}

std::unique_ptr<Value> ValueBuilder::value( EscriptParser::LiteralContext* ctx )
{
  if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    return string_value( string_literal );
  }
  else if ( auto integer_literal = ctx->integerLiteral() )
  {
    return integer_value( integer_literal );
  }
  else if ( auto float_literal = ctx->floatLiteral() )
  {
    return float_value( float_literal );
  }
  else if ( auto bool_literal = ctx->boolLiteral() )
  {
    return bool_value( bool_literal );
  }
  else if ( ctx->UNINIT() )
  {
    return std::make_unique<UninitializedValue>( location_for( *ctx ) );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled literal" );
  }
}

int ValueBuilder::to_int( EscriptParser::IntegerLiteralContext* ctx )
{
  try
  {
    if ( auto decimal_literal = ctx->DECIMAL_LITERAL() )
    {
      return std::stoi( decimal_literal->getSymbol()->getText() );
    }
    else if ( auto hex_literal = ctx->HEX_LITERAL() )
    {
      return static_cast<int>( std::stoul( hex_literal->getSymbol()->getText(), nullptr, 16 ) );
    }
    else if ( auto oct_literal = ctx->OCT_LITERAL() )
    {
      return std::stoi( oct_literal->getSymbol()->getText(), nullptr, 8 );
    }
    else if ( auto binary_literal = ctx->BINARY_LITERAL() )
    {
      return std::stoi( binary_literal->getSymbol()->getText(), nullptr, 2 );
    }
  }
  catch ( std::invalid_argument& )
  {
    report.error( location_for( *ctx ), "unable to convert integer value '{}'.", ctx->getText() );
    throw;
  }
  catch ( std::out_of_range& )
  {
    report.error( location_for( *ctx ), "integer value '{}' out of range.", ctx->getText() );
    throw;
  }

  location_for( *ctx ).internal_error( "unhandled integer literal" );
}

}  // namespace Pol::Bscript::Compiler
