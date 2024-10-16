#include "TreeBuilder.h"

#include "bscript/compiler/Antlr4Inc.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
TreeBuilder::TreeBuilder( const SourceFileIdentifier& source_file_identifier,
                          BuilderWorkspace& workspace )
  : report( workspace.report ),
    source_file_identifier( source_file_identifier ),
    workspace( workspace )
{
}

std::unique_ptr<Expression> TreeBuilder::consume_expression_result(
    std::unique_ptr<Expression> expression )
{
  return std::make_unique<ValueConsumer>( expression->source_location, std::move( expression ) );
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
  if ( !terminal )
    return "";
  return terminal->getSymbol()->getText();
}

std::string TreeBuilder::text( antlr4::Token* token )
{
  return token->getText();
}

}  // namespace Pol::Bscript::Compiler
