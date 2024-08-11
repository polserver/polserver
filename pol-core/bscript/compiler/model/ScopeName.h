#pragma once

#include <optional>
#include <string>

namespace Pol::Bscript::Compiler
{

class ScopeName : private std::optional<std::string>
{
public:
  ScopeName( const std::string& name );
  ScopeName();

  // True if scope is non-global scope
  bool exists() const;

  // True if scope was not specified
  bool empty() const;

  // Returns "" or "foo"
  std::string string() const;

  static ScopeName Global;
  static ScopeName None;
};
}  // namespace Pol::Bscript::Compiler
