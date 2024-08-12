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

  // True if scope is global scope
  bool global() const;

  // True if scope was not specified
  bool empty() const;

  // Returns "" or "foo"
  std::string string() const;

  static ScopeName Global;
  static ScopeName None;

  bool operator<( const ScopeName& other ) const;
};
}  // namespace Pol::Bscript::Compiler
