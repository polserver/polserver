#include "TreeBuilder.h"

#include <antlr4-runtime.h>

#include "BuilderWorkspace.h"
#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
TreeBuilder::TreeBuilder( const SourceFileIdentifier& source_file_identifier,
                          BuilderWorkspace& workspace )
  : report( workspace.report ),
    source_file_identifier( source_file_identifier ),
    workspace( workspace )
{
}

SourceLocation TreeBuilder::location_for( antlr4::ParserRuleContext& ctx ) const
{
  return SourceLocation( &source_file_identifier, ctx );
}

SourceLocation TreeBuilder::location_for( antlr4::tree::TerminalNode& ctx ) const
{
  return SourceLocation( &source_file_identifier, ctx );
}

std::string TreeBuilder::text( antlr4::tree::TerminalNode* terminal )
{
  return terminal->getSymbol()->getText();
}

}  // namespace Pol::Bscript::Compiler
