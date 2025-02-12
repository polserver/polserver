#include "MemberUnpacking.h"

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::string scope, std::string name,
                                  std::unique_ptr<Expression> member )
    : Node( loc, std::move( member ) ),
      scoped_name( ScopableName( std::move( scope ), std::move( name ) ) ),
      rest( false )
{
}

MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::string scope, std::string name )
    : Node( loc ),
      scoped_name( ScopableName( std::move( scope ), std::move( name ) ) ),
      rest( true )
{
}

MemberUnpacking::MemberUnpacking( const SourceLocation& loc, std::unique_ptr<Expression> member,
                                  std::unique_ptr<Node> unpacking )
    : Node( loc ), scoped_name( std::nullopt ), rest( false )
{
  children.reserve( 2 );
  children.push_back( std::move( member ) );
  children.push_back( std::move( unpacking ) );
}

void MemberUnpacking::accept( NodeVisitor& visitor )
{
  visitor.visit_member_unpacking( *this );
}

void MemberUnpacking::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "member-unpacking({}{})",
                  scoped_name.has_value() ? scoped_name.value().string() : "", rest ? "..." : "" );
}

}  // namespace Pol::Bscript::Compiler
