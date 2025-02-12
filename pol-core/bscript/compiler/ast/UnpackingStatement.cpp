#include "UnpackingStatement.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UnpackingStatement::UnpackingStatement( const SourceLocation& source_location,
                                        std::unique_ptr<Node> unpackings,
                                        std::unique_ptr<Expression> initializer )
    : Statement( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( unpackings ) );
  children.push_back( std::move( initializer ) );
}
void UnpackingStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_unpacking_statement( *this );
}

void UnpackingStatement::describe_to( std::string& w ) const
{
  w += "unpacking-statement";
}

}  // namespace Pol::Bscript::Compiler
