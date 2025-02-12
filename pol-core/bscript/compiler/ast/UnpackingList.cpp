#include "UnpackingList.h"

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UnpackingList::UnpackingList( const SourceLocation& loc,
                              std::vector<std::unique_ptr<Node>> unpackings, bool index_unpacking )
    : Node( loc, std::move( unpackings ) ), index_unpacking( index_unpacking )
{
}
void UnpackingList::accept( NodeVisitor& visitor )
{
  visitor.visit_unpacking_list( *this );
}

void UnpackingList::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "unpacking-list({})",
                  index_unpacking ? "index" : "member" );
}

}  // namespace Pol::Bscript::Compiler
