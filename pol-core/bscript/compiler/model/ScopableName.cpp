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

ScopableName::ScopableName( ScopableName&& other ) noexcept
    : scope( std::move( other.scope ) ), name( std::move( other.name ) )
{
}

ScopableName::ScopableName( const ScopableName& other ) : scope( other.scope ), name( other.name )
{
}

bool ScopableName::scoped() const
{
  return scope.exists();
}

std::string ScopableName::string() const
{
  return fmt::format( "{}{}{}", scope.string(), scoped() ? "::" : "", name );
}

std::string ScopableName::maybe_scoped_string() const
{
  return fmt::format( "{}{}{}", scope.string(), scope.exists() ? "::" : "", name );
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
  else if ( scope.exists() && other.scope.exists() )
  {
    // Current object has scope, other doesn't
    return false;  // Current object is greater
  }
  else if ( !scope.exists() && other.scope.exists() )
  {
    // Current object doesn't have scope, other does
    return true;  // Current object is less
  }

  // If scopes are equal (both null or both equal), compare names
  return stricmp( name.c_str(), other.name.c_str() ) < 0;
}

}  // namespace Pol::Bscript::Compiler
