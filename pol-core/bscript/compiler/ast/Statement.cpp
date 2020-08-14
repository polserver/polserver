#include "Statement.h"

namespace Pol::Bscript::Compiler
{
Statement::Statement( const SourceLocation& source_location ) : Node( source_location ) {}

Statement::Statement( const SourceLocation& source_location, std::unique_ptr<Node> child )
  : Node( source_location, std::move( child ) )
{
}

Statement::Statement( const SourceLocation& source_location, NodeVector children )
  : Node( source_location, std::move( children ) )
{
}

}  // namespace Pol::Bscript::Compiler
