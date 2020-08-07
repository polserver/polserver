#include "GetMember.h"

#include <format/format.h>

#include "NodeVisitor.h"
#include "compilercfg.h"

namespace Pol::Bscript::Compiler
{
GetMember::GetMember( const SourceLocation& source_location, std::unique_ptr<Expression> lhs,
                      std::string name )
    : Expression( source_location, std::move( lhs ) ),
      name( std::move( name ) ),
      known_member( compilercfg.OptimizeObjectMembers
                        ? Pol::Bscript::getKnownObjMember( this->name.c_str() )
                        : nullptr )
{
}

void GetMember::accept( NodeVisitor& visitor )
{
  visitor.visit_get_member( *this );
}

void GetMember::describe_to( fmt::Writer& w ) const
{
  w << "get-member(" << name << ")";
}

std::unique_ptr<Expression> GetMember::take_entity()
{
  return take_child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
