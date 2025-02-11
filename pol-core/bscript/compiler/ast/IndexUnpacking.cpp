#include "IndexUnpacking.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
IndexUnpacking::IndexUnpacking( const SourceLocation& loc, std::string name, bool rest )
    : Node( loc ), name( std::move( name ) ), rest( rest )
{
}
void IndexUnpacking::accept( NodeVisitor& ) {}

void IndexUnpacking::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "index-unpacking({}{})", name, rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
