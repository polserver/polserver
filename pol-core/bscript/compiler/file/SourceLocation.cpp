#include "SourceLocation.h"

#include "bscript/compiler/Antlr4Inc.h"

#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "clib/filecont.h"
#include "clib/logfacility.h"

#include <iterator>

#include <climits>

namespace Pol::Bscript::Compiler
{
Position calculate_end_position( const antlr4::Token* symbol )
{
  if ( !symbol )
  {
    return Position{ 0, 0, 0 };
  }
  auto line = symbol->getLine();
  auto character = symbol->getCharPositionInLine() + 1;
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

  return Position{ line, character, symbol->getTokenIndex() };
}

Range::Range( Position start, Position end ) : start( std::move( start ) ), end( std::move( end ) )
{
}

Range::Range( const antlr4::ParserRuleContext& ctx )
    : start( Position{ ctx.getStart()->getLine(), ctx.getStart()->getCharPositionInLine() + 1,
                       ctx.getStart()->getTokenIndex() } ),
      end( calculate_end_position( ctx.getStop() ) )
{
}

Range::Range( const antlr4::tree::TerminalNode& ctx )
    : start( Position{ ctx.getSymbol()->getLine(), ctx.getSymbol()->getCharPositionInLine() + 1,
                       ctx.getSymbol()->getTokenIndex() } ),
      end( calculate_end_position( ctx.getSymbol() ) )
{
}

Range::Range( const antlr4::Token* token )
    : start( Position{ token->getLine(), token->getCharPositionInLine() + 1,
                       token->getTokenIndex() } ),
      end( calculate_end_position( token ) )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                size_t line_number, size_t character_column )
    : source_file_identifier( source_file_identifier ),
      range( Position{ line_number, character_column, 0 },
             Position{ std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max(),
                       std::numeric_limits<size_t>::max() } )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                const Range& range )
    : source_file_identifier( source_file_identifier ), range( range )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                const antlr4::ParserRuleContext& ctx )
    : source_file_identifier( source_file_identifier ), range( ctx )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                const antlr4::tree::TerminalNode& ctx )
    : source_file_identifier( source_file_identifier ), range( ctx )
{
}

bool Range::contains( const Position& position ) const
{
  return contains( position.line_number, position.character_column );
}

bool Range::contains( size_t line_number, size_t character_column ) const
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
  ERROR_PRINTLN( "{}: {}", ( *this ), msg );
}

void SourceLocation::internal_error( const std::string& msg ) const
{
  ERROR_PRINTLN( "{}: {}", ( *this ), msg );
  throw std::runtime_error( msg );
}

void SourceLocation::internal_error( const std::string& msg, const SourceLocation& related ) const
{
  ERROR_PRINTLN( "{}: {}\n  See also: {}", ( *this ), msg, related );
  throw std::runtime_error( msg );
}

std::string SourceLocation::getSourceLine() const
{
  std::string lines;
  if ( !range.start.line_number )
    return {};
  auto& content = source_file_identifier->getLines();
  for ( size_t i = range.start.line_number - 1; i < range.end.line_number && i < content.size();
        ++i )
  {
    if ( lines.size() )
      lines += '\n';
    lines += content[i];
  }

  return lines;
}

}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::SourceLocation>::format(
    const Pol::Bscript::Compiler::SourceLocation& l, fmt::format_context& ctx ) const
{
  std::string tmp = l.source_file_identifier->pathname;
  if ( l.range.start.line_number || l.range.start.character_column )
    fmt::format_to( std::back_inserter( tmp ), ":{}:{}", l.range.start.line_number,
                    l.range.start.character_column );
  return fmt::formatter<std::string>::format( tmp, ctx );
}
