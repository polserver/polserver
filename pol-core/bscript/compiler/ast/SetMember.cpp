#include "SetMember.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
SetMember::SetMember( const SourceLocation& source_location, bool consume,
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

void SetMember::accept( NodeVisitor& visitor )
{
  visitor.visit_set_member( *this );
}

void SetMember::describe_to( fmt::Writer& w ) const
{
  w << "set-member";
  if ( consume )
    w << "-consume";
  w << "(" << name << ")";
  if ( known_member )
    w << " (#" << known_member->id << ")";
}

Expression& SetMember::entity()
{
  return child<Expression>( 0 );
}

Expression& SetMember::rhs()
{
  return child<Expression>( 1 );
}

std::unique_ptr<Expression> SetMember::take_entity()
{
  return take_child<Expression>( 0 );
}

std::unique_ptr<Expression> SetMember::take_rhs()
{
  return take_child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler