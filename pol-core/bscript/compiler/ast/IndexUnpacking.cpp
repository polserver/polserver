#include "IndexUnpacking.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
IndexUnpacking::IndexUnpacking( const SourceLocation& loc, std::string scope, std::string name,
                                bool rest )
    : Node( loc ), scoped_name( std::move( scope ), std::move( name ) ), rest( rest ), variable()
{
}

void IndexUnpacking::accept( NodeVisitor& visitor )
{
  visitor.visit_index_unpacking( *this );
}

void IndexUnpacking::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "index-unpacking({}{})", scoped_name.string(),
                  rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
