#include "IndexBinding.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
IndexBinding::IndexBinding( const SourceLocation& loc, std::string name, bool rest )
    : Node( loc ), name( std::move( name ) ), rest( rest )
{
}
void IndexBinding::accept( NodeVisitor& ) {}

void IndexBinding::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "index-binding({}{})", name, rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
