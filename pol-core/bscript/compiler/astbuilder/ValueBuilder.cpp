#include "ValueBuilder.h"

#include <cstring>

#include "clib/strutil.h"
#include "compiler/Report.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/StringValue.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/file/SourceLocation.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ValueBuilder::ValueBuilder( const SourceFileIdentifier& source_file_identifier,
                            BuilderWorkspace& workspace )
  : TreeBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<FloatValue> ValueBuilder::float_value(
    EscriptGrammar::EscriptParser::FloatLiteralContext* ctx )
{
  auto loc = location_for( *ctx );
  double value = std::stod( text( ctx->FLOAT_LITERAL() ) );
  return std::make_unique<FloatValue>( loc, value );
}

std::unique_ptr<IntegerValue> ValueBuilder::integer_value(
    EscriptParser::IntegerLiteralContext* ctx )
{
  auto loc = location_for( *ctx );
  return std::make_unique<IntegerValue>( loc, to_int( ctx ) );
}

std::unique_ptr<StringValue> ValueBuilder::string_value(
    antlr4::tree::TerminalNode* string_literal )
{
  auto loc = location_for( *string_literal );
  return std::make_unique<StringValue>( loc, unquote( string_literal ) );
}

std::string ValueBuilder::unquote( antlr4::tree::TerminalNode* string_literal )
{
  std::string input = string_literal->getSymbol()->getText();
  const char* s = input.c_str();
  if ( *s != '\"' )
    location_for( *string_literal ).internal_error( "string does not begin with a quote?" );

  const char* end = s + 1;
  std::string lit;
  lit.reserve(input.length());
  bool escnext = false;  // true when waiting for 2nd char in an escape sequence
  u8 hexnext = 0;        // tells how many more chars in a \xNN escape sequence
  char hexstr[3];        // will contain the \x escape chars to be processed
  memset( hexstr, 0, 3 );

  for ( ;; )
  {
    if ( !*end )
    {
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
          report.error( location_for( *string_literal ), "Invalid hex escape sequence '", hexstr,
                        "'.\n" );
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
                    "Warning: invalid unicode character detected. Assuming ISO8859.\n" );

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
  else
  {
    location_for( *ctx ).internal_error( "unhandled literal" );
  }
}

int ValueBuilder::to_int( EscriptParser::IntegerLiteralContext* ctx )
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
  location_for( *ctx ).internal_error( "unhandled integer literal" );
}

}  // namespace Pol::Bscript::Compiler
