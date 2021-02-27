#include "MemberAssignmentByOperator.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MemberAssignmentByOperator::MemberAssignmentByOperator(
    const SourceLocation& source_location, bool consume, std::unique_ptr<Expression> entity,
    std::string name, BTokenId token_id, std::unique_ptr<Expression> rhs,
    const Pol::Bscript::ObjMember& known_member )
    : Expression( source_location ),
      consume( consume ),
      name( std::move( name ) ),
      token_id( token_id ),
      known_member( known_member )
{
  children.reserve( 2 );
  children.push_back( std::move( entity ) );
  children.push_back( std::move( rhs ) );
}

MemberAssignmentByOperator::MemberAssignmentByOperator(
    const SourceLocation& source_location, bool consume, std::unique_ptr<Expression> entity,
    std::string name, BTokenId token_id, const Pol::Bscript::ObjMember& known_member )
    : Expression( source_location ),
      consume( consume ),
      name( std::move( name ) ),
      token_id( token_id ),
      known_member( known_member )
{
  children.reserve( 1 );
  children.push_back( std::move( entity ) );
}

void MemberAssignmentByOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_member_assignment_by_operator( *this );
}

void MemberAssignmentByOperator::describe_to( fmt::Writer& w ) const
{
  w << "member-assignment-by-operator(" << name << ", " << token_id << ")";
}

}  // namespace Pol::Bscript::Compiler
