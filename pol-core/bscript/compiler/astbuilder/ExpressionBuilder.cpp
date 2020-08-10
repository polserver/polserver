#include "ExpressionBuilder.h"

#include "compiler/Report.h"
#include "compiler/ast/Value.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<Expression> ExpressionBuilder::expression( EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return primary( prim );

  location_for( *ctx ).internal_error( "unhandled expression" );
}

std::unique_ptr<Expression> ExpressionBuilder::primary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
    return value( literal );

  location_for( *ctx ).internal_error( "unhandled primary expression" );
}

}  // namespace Pol::Bscript::Compiler
