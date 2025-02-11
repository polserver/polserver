#include "MemberBinding.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MemberBinding::MemberBinding( const SourceLocation& loc, std::string name,
                              std::unique_ptr<Expression> member )
    : Node( loc, std::move( member ) ), name( std::move( name ) ), rest( false )
{
}

MemberBinding::MemberBinding( const SourceLocation& loc, std::string name )
    : Node( loc ), name( std::move( name ) ), rest( true )
{
}

MemberBinding::MemberBinding( const SourceLocation& loc, std::unique_ptr<Expression> member,
                              std::unique_ptr<Node> binding )
    : Node( loc )
{
  children.reserve( 2 );
  children.push_back( std::move( member ) );
  children.push_back( std::move( binding ) );
}
void MemberBinding::accept( NodeVisitor& ) {}

void MemberBinding::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "member-binding({}{})", name, rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
