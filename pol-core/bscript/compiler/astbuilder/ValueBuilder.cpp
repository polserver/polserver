#include "ValueBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/FloatValue.h"
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
  double value = std::stod( ctx->FLOAT_LITERAL()->getSymbol()->getText() );
  return std::make_unique<FloatValue>( loc, value );
}


std::unique_ptr<Value> ValueBuilder::value( EscriptParser::LiteralContext* ctx )
{
  if ( auto float_literal = ctx->floatLiteral() )
  {
    return float_value( float_literal );
  }
  else
  {
    location_for( *ctx ).internal_error( "unhandled literal" );
  }
}

}  // namespace Pol::Bscript::Compiler
