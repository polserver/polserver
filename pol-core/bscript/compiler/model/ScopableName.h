#pragma once

#include "bscript/compiler/model/ScopeName.h"

namespace Pol::Bscript::Compiler
{
class ScopableName
{
public:
  ScopableName( const ScopeName& scope, const std::string& name );
  ScopableName( const std::string& scope, const std::string& name );
  ScopableName( ScopableName&& other ) noexcept;
  ScopableName( const ScopableName& );

  // True if the scope is non-empty.
  bool scoped() const;

  // Returns ::foo, foo, Animal::foo
  std::string string() const;

  // Returns foo, Animal::foo. Should only be used in displaying messages to
  // users, otherwise there will be an ambiguity between foo and ::foo.
  std::string maybe_scoped_string() const;

  bool operator<( const ScopableName& other ) const;

  ScopeName scope;
  std::string name;
};
}  // namespace Pol::Bscript::Compiler
