#include "SetMemberByOperator.h"

#include <format/format.h>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
SetMemberByOperator::SetMemberByOperator( const SourceLocation& source_location, bool consume,
                                          std::unique_ptr<Expression> entity, std::string name,
                                          BTokenId token_id, std::unique_ptr<Expression> rhs,
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

SetMemberByOperator::SetMemberByOperator( const SourceLocation& source_location, bool consume,
                                          std::unique_ptr<Expression> entity, std::string name,
                                          BTokenId token_id,
                                          const Pol::Bscript::ObjMember& known_member )
    : Expression( source_location ),
      consume( consume ),
      name( std::move( name ) ),
      token_id( token_id ),
      known_member( known_member )
{
  children.reserve( 1 );
  children.push_back( std::move( entity ) );
}

void SetMemberByOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_set_member_by_operator( *this );
}

void SetMemberByOperator::describe_to( fmt::Writer& w ) const
{
  w << "set-member-by-operator(" << name << ", " << token_id << ")";
}

}  // namespace Pol::Bscript::Compiler
