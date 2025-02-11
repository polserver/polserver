#include "BindingList.h"

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BindingList::BindingList( const SourceLocation& loc, std::vector<std::unique_ptr<Node>> bindings,
                          bool index_binding )
    : Node( loc, std::move( bindings ) ), index_binding( index_binding )
{
}
void BindingList::accept( NodeVisitor& ) {}

void BindingList::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "binding-list({})", index_binding ? "index" : "member" );
}

}  // namespace Pol::Bscript::Compiler
