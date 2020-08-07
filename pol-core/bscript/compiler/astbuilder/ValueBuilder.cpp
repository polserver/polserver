#include "ValueBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/StringValue.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionReference.h"
#include "compiler/ast/StringValue.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/FunctionLink.h"

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
  double value = std::stod( ctx->FLOAT_LITERAL()->getSymbol()->getText() );
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
  else if ( auto float_literal = ctx->floatLiteral() )
  {
    return float_value( float_literal );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled literal" );
  }
}

}  // namespace Pol::Bscript::Compiler
