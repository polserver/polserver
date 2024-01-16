#include "SourceLocation.h"

#include "bscript/compiler/Antlr4Inc.h"

#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "clib/logfacility.h"

#include <iterator>

namespace Pol::Bscript::Compiler
{
SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                unsigned short line_number, unsigned short character_column )
    : source_file_identifier( source_file_identifier ),
      line_number( line_number ),
      character_column( character_column )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::ParserRuleContext& ctx )
    : source_file_identifier( source_file_identifier ),
      line_number( static_cast<unsigned short>( ctx.getStart()->getLine() ) ),
      character_column( static_cast<unsigned short>( ctx.getStart()->getCharPositionInLine() + 1 ) )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::tree::TerminalNode& ctx )
    : source_file_identifier( source_file_identifier ),
      line_number( static_cast<unsigned short>( ctx.getSymbol()->getLine() ) ),
      character_column(
          static_cast<unsigned short>( ctx.getSymbol()->getCharPositionInLine() + 1 ) )
{
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

}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::SourceLocation>::format(
    const Pol::Bscript::Compiler::SourceLocation& l, fmt::format_context& ctx ) const
{
  std::string tmp = l.source_file_identifier->pathname;
  if ( l.line_number || l.character_column )
    fmt::format_to( std::back_inserter( tmp ), ":{}:{}", l.line_number, l.character_column );
  return fmt::formatter<std::string>::format( tmp, ctx );
}
