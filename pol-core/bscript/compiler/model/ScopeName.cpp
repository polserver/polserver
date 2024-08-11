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
}  // namespace Pol::Bscript::Compiler
