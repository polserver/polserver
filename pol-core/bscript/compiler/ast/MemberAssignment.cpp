#include "MemberAssignment.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MemberAssignment::MemberAssignment( const SourceLocation& source_location, bool consume,
                                    std::unique_ptr<Expression> entity, std::string name,
                                    std::unique_ptr<Expression> rhs,
                                    const Pol::Bscript::ObjMember* const known_member )
    : Expression( source_location ),
      consume( consume ),
      name( std::move( name ) ),
      known_member( known_member )
{
  children.reserve( 2 );
  children.push_back( std::move( entity ) );
  children.push_back( std::move( rhs ) );
}

void MemberAssignment::accept( NodeVisitor& visitor )
{
  visitor.visit_member_assignment( *this );
}

void MemberAssignment::describe_to( std::string& w ) const
{
  w += "member-assignment";
  if ( consume )
    w += "-statement";
  fmt::format_to( std::back_inserter( w ), "({})", name );
  if ( known_member )
    fmt::format_to( std::back_inserter( w ), " (#{})", known_member->id );
}

Expression& MemberAssignment::entity()
{
  return child<Expression>( 0 );
}

Expression& MemberAssignment::rhs()
{
  return child<Expression>( 1 );
}

std::unique_ptr<Expression> MemberAssignment::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<Expression> MemberAssignment::take_rhs()
{
  return take_child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler