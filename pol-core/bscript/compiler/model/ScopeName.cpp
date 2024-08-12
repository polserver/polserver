#include "ScopeName.h"

#include "clib/clib.h"

namespace Pol::Bscript::Compiler
{
ScopeName ScopeName::Global = ScopeName( "" );
ScopeName ScopeName::None = ScopeName();

ScopeName::ScopeName( const std::string& name ) : std::optional<std::string>( name ) {}

ScopeName::ScopeName() : std::optional<std::string>( std::nullopt ) {}

bool ScopeName::global() const
{
  return empty() || value().empty();
}

bool ScopeName::empty() const
{
  return !has_value();
}

std::string ScopeName::string() const
{
  return value_or( "" );
}
bool ScopeName::operator<( const ScopeName& other ) const
{
  if ( !has_value() && !other.has_value() )
  {
    return false;
  }
  if ( !has_value() && other.has_value() )
  {
    return true;
  }
  if ( has_value() && !other.has_value() )
  {
    return false;
  }
  return stricmp( value().c_str(), other.value().c_str() ) < 0;
}
}  // namespace Pol::Bscript::Compiler
