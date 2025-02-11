#include "BindingStatement.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BindingStatement::BindingStatement( const SourceLocation& source_location,
                                    std::unique_ptr<Node> bindings,
                                    std::unique_ptr<Expression> initializer )
    : Statement( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( bindings ) );
  children.push_back( std::move( initializer ) );
}
void BindingStatement::accept( NodeVisitor& ) {}

void BindingStatement::describe_to( std::string& w ) const
{
  w += "binding-statement";
}

}  // namespace Pol::Bscript::Compiler
