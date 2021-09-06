#include "SourceLocation.h"

#include "bscript/compiler/Antlr4Inc.h"

#include "clib/logfacility.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"

#include <climits>

namespace Pol::Bscript::Compiler
{
Position calculate_end_position( antlr4::Token* symbol )
{
  if ( !symbol )
  {
    return Position{ 0, 0 };
  }
  auto line = static_cast<unsigned short>( symbol->getLine() );
  auto character = static_cast<unsigned short>( symbol->getCharPositionInLine() + 1 );
  const auto& str = symbol->getText();
  const auto size = str.size();

  for ( size_t i = 0; i < size; i++ )
  {
    if ( str[i] == '\r' && i + 1 < size && str[i + 1] == '\n' )
    {
      ++line;
      character = 1;
    }
    else if ( str[i] == '\r' )
    {
      ++line;
      character = 1;
    }
    else if ( str[i] == '\n' )
    {
      ++line;
      character = 1;
    }
    else
      ++character;
  }

  return Position{ line, character };
}

Range::Range( const Position start, const Position end ) : start( start ), end( end ) {}

Range::Range( antlr4::ParserRuleContext& ctx )
    : start(
          Position{ static_cast<unsigned short>( ctx.getStart()->getLine() ),
                    static_cast<unsigned short>( ctx.getStart()->getCharPositionInLine() + 1 ) } ),
      end( calculate_end_position( ctx.getStop() ) )
{
}

Range::Range( antlr4::tree::TerminalNode& ctx )
    : start(
          Position{ static_cast<unsigned short>( ctx.getSymbol()->getLine() ),
                    static_cast<unsigned short>( ctx.getSymbol()->getCharPositionInLine() + 1 ) } ),
      end( calculate_end_position( ctx.getSymbol() ) )
{
}

Range::Range( antlr4::Token* token )
    : start( Position{ static_cast<unsigned short>( token->getLine() ),
                       static_cast<unsigned short>( token->getCharPositionInLine() + 1 ) } ),
      end( calculate_end_position( token ) )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                unsigned short line_number, unsigned short character_column )
    : source_file_identifier( source_file_identifier ),
      range( Position{ line_number, character_column }, Position{ USHRT_MAX, USHRT_MAX } )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::ParserRuleContext& ctx )
    : source_file_identifier( source_file_identifier ), range( ctx )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::tree::TerminalNode& ctx )
    : source_file_identifier( source_file_identifier ), range( ctx )
{
}

bool Range::contains( const Position& position ) const
{
  return contains( position.line_number, position.character_column );
}

bool Range::contains( unsigned short line_number, unsigned short character_column ) const
{
  if ( line_number < start.line_number || line_number > end.line_number )
  {
    return false;
  }
  if ( line_number == start.line_number && character_column < start.character_column )
  {
    return false;
  }
  if ( line_number == end.line_number && character_column > end.character_column )
  {
    return false;
  }
  return true;
}

bool Range::contains( const Range& otherRange ) const
{
  if ( otherRange.start.line_number < start.line_number ||
       otherRange.end.line_number < start.line_number )
  {
    return false;
  }
  if ( otherRange.start.line_number > end.line_number ||
       otherRange.end.line_number > end.line_number )
  {
    return false;
  }
  if ( otherRange.start.line_number == start.line_number &&
       otherRange.start.character_column < start.character_column )
  {
    return false;
  }
  if ( otherRange.end.line_number == end.line_number &&
       otherRange.end.character_column > end.character_column )
  {
    return false;
  }
  return true;
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
  w << location.source_file_identifier->pathname << location.range.start;
  return w;
}

}  // namespace Pol::Bscript::Compiler
