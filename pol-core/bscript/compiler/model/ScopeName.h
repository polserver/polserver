#pragma once

#include <optional>
#include <string>

namespace Pol::Bscript::Compiler
{
const std::string SUPER = "super";

class ScopeName : private std::optional<std::string>
{
public:
  ScopeName( std::string name );
  ScopeName();

  // True if scope is global scope
  bool global() const;

  // True if scope is super scope
  bool super() const;

  // True if scope was not specified
  bool empty() const;

  // Returns "" or "foo"
  std::string string() const;

  static ScopeName Global;
  static ScopeName None;
  static ScopeName Super;

  bool operator<( const ScopeName& other ) const;
};
}  // namespace Pol::Bscript::Compiler
