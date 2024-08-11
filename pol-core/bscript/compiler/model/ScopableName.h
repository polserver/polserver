#pragma once

#include "bscript/compiler/model/ScopeName.h"

namespace Pol::Bscript::Compiler
{
class ScopableName
{
public:
  ScopableName( const ScopeName& scope, const std::string& name );
  ScopableName( const std::string& scope, const std::string& name );

  // True if the scope is global.
  bool global() const;

  // Returns foo, Animal::foo but never ::foo
  std::string string() const;

  bool operator<( const ScopableName& other ) const;

  ScopeName scope;
  std::string name;
};
}  // namespace Pol::Bscript::Compiler
