#include "MemberUnpacking.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::string name,
                                  std::unique_ptr<Expression> member )
    : Node( loc, std::move( member ) ), name( std::move( name ) ), rest( false )
{
}

MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::string name )
    : Node( loc ), name( std::move( name ) ), rest( true )
{
}

MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::unique_ptr<Expression> member,
                                  std::unique_ptr<Node> unpacking )
    : Node( loc )
{
  children.reserve( 2 );
  children.push_back( std::move( member ) );
  children.push_back( std::move( unpacking ) );
}
void MemberUnpacking::accept( NodeVisitor& ) {}

void MemberUnpacking::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "member-unpacking({}{})", name, rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
