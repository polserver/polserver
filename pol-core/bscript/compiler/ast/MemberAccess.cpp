#include "MemberAccess.h"


#include "bscript/compiler/ast/NodeVisitor.h"
#include "compilercfg.h"

namespace Pol::Bscript::Compiler
{
MemberAccess::MemberAccess( const SourceLocation& source_location, std::unique_ptr<Expression> lhs,
                            std::string name )
    : Expression( source_location, std::move( lhs ) ),
      name( std::move( name ) ),
      known_member( compilercfg.OptimizeObjectMembers
                        ? Pol::Bscript::getKnownObjMember( this->name.c_str() )
                        : nullptr )
{
}

void MemberAccess::accept( NodeVisitor& visitor )
{
  visitor.visit_member_access( *this );
}

void MemberAccess::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "{}({})", type(), name );
}

std::string MemberAccess::type() const
{
  return "member-access";
}

std::unique_ptr<Expression> MemberAccess::take_entity()
{
  return take_child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
