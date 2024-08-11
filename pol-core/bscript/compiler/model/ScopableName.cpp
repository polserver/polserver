#include "ScopableName.h"

#include <fmt/format.h>

#include "clib/clib.h"

namespace Pol::Bscript::Compiler
{
ScopableName::ScopableName( const ScopeName& scope, const std::string& name )
    : scope( scope ), name( name )
{
}

ScopableName::ScopableName( const std::string& scope, const std::string& name )
    : ScopableName( ScopeName( scope ), name )
{
}

bool ScopableName::global() const
{
  return scope.global();
}

std::string ScopableName::string() const
{
  return fmt::format( "{}{}{}", scope.string(), scope.global() ? "" : "::", name );
}

bool ScopableName::operator<( const ScopableName& other ) const
{
  // Compare scopes first
  if ( !scope.empty() && !other.scope.empty() )
  {
    // Both scopes are present, compare them using stricmp
    int scopeComparison = stricmp( scope.string().c_str(), other.scope.string().c_str() );
    if ( scopeComparison < 0 )
    {
      return true;
    }
    if ( scopeComparison > 0 )
    {
      return false;
    }
  }
  else if ( !scope.empty() && other.scope.empty() )
  {
    // Current object doesn't have scope, other does
    return false;  // Current object is greater
  }
  else if ( scope.empty() && !other.scope.empty() )
  {
    // Current object has scope, other doesn't
    return true;  // Current object is less
  }

  // If scopes are equal (both null or both equal), compare names
  return stricmp( name.c_str(), other.name.c_str() ) < 0;
}

}  // namespace Pol::Bscript::Compiler
