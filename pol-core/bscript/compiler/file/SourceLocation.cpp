#include "SourceLocation.h"

#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "clib/logfacility.h"
#include <antlr4-runtime.h>
#include <climits>

namespace Pol::Bscript::Compiler
{
Position calculate_end_position( antlr4::Token* symbol )
{
  unsigned short line = symbol->getLine();
  unsigned short character = symbol->getCharPositionInLine() + 1;
  const auto& str = symbol->getText();
  for ( size_t i = 0; i < str.size(); i++ )
  {
    if ( str[i] == '\r' && str[i + 1] == '\n' )
      ++line, character = 1;
    else if ( str[i] == '\r' )
      ++line, character = 1;
    else if ( str[i] == '\n' )
      ++line, character = 1;
    else
      ++character;
  }
  return Position{ line, character };
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                unsigned short line_number, unsigned short character_column )
    : source_file_identifier( source_file_identifier ),
      start( Position{ line_number, character_column } ),
      end( Position{ USHRT_MAX, USHRT_MAX } )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::ParserRuleContext& ctx )
    : source_file_identifier( source_file_identifier ),
      start(
          Position{ static_cast<unsigned short>( ctx.getStart()->getLine() ),
                    static_cast<unsigned short>( ctx.getStart()->getCharPositionInLine() + 1 ) } ),
      end( calculate_end_position( ctx.getStop() ) )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::tree::TerminalNode& ctx )
    : source_file_identifier( source_file_identifier ),
      start(
          Position{ static_cast<unsigned short>( ctx.getSymbol()->getLine() ),
                    static_cast<unsigned short>( ctx.getSymbol()->getCharPositionInLine() + 1 ) } ),
      end( calculate_end_position( ctx.getSymbol() ) )
{
}

void SourceLocation::debug( const std::string& msg ) const
{
  ERROR_PRINT << ( *this ) << ": " << msg << "\n";
}

void SourceLocation::internal_error( const std::string& msg ) const
{
  ERROR_PRINT << ( *this ) << ": " << msg << "\n";
  throw std::runtime_error( msg );
}

void SourceLocation::internal_error( const std::string& msg, const SourceLocation& related ) const
{
  ERROR_PRINT << ( *this ) << ": " << msg << "\n"
              << "  See also: " << related << "\n";
  throw std::runtime_error( msg );
}

fmt::Writer& operator<<( fmt::Writer& w, const Position& position )
{
  if ( position.line_number || position.character_column )
  {
    w << ":" << position.line_number << ":" << position.character_column;
  }
  return w;
}

fmt::Writer& operator<<( fmt::Writer& w, const SourceLocation& location )
{
  w << location.source_file_identifier->pathname << location.start;
  return w;
}

}  // namespace Pol::Bscript::Compiler
