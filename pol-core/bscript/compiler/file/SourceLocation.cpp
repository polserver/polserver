#include "SourceLocation.h"

#include <antlr4-runtime.h>

#include "../clib/logfacility.h"

#include "compiler/file/SourceFileIdentifier.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                unsigned short line_number,
                                unsigned short character_column )
  : source_file_identifier( source_file_identifier ),
    line_number( line_number ),
    character_column( character_column )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::ParserRuleContext& ctx )
  : source_file_identifier( source_file_identifier ),
    line_number( static_cast<unsigned short>( ctx.getStart()->getLine() ) ),
    character_column( static_cast<unsigned short>( ctx.getStart()->getCharPositionInLine()+1 ) )
{
}

SourceLocation::SourceLocation( const SourceFileIdentifier* source_file_identifier,
                                antlr4::tree::TerminalNode& ctx )
  : source_file_identifier( source_file_identifier ),
    line_number( static_cast<unsigned short>( ctx.getSymbol()->getLine() ) ),
    character_column( static_cast<unsigned short>( ctx.getSymbol()->getCharPositionInLine()+1 ) )
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

fmt::Writer& operator<<( fmt::Writer& w, const SourceLocation& location )
{
  w << location.source_file_identifier->pathname;
  if ( location.line_number || location.character_column )
    w << ":" << location.line_number << ":" << location.character_column;
  return w;
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
