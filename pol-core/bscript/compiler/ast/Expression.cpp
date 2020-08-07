#include "Expression.h"

namespace Pol::Bscript::Compiler
{
Expression::Expression( const SourceLocation& source_location ) : Statement( source_location ) {}

Expression::Expression( const SourceLocation& source_location, std::unique_ptr<Node> child )
  : Statement( source_location, std::move( child ) )
{
}

Expression::Expression( const SourceLocation& source_location, NodeVector children )
  : Statement( source_location, std::move( children ) )
{
}

}  // namespace Pol::Bscript::Compiler