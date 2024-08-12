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

  // `this` ScopableName is equal to `other` if `this` is referencing `other` as
  // a constructor, ie. if <nullopt>::Foo == Foo::Foo.
  //
  // Since _only_ the FunctionResolver uses a container of ScopableNames, this
  // specific behavior works. If using inside any different context, this may
  // need to change. Used in a maps for:
  //
  // - unresolved function calls to their linkes, eg:
  //
  //      <nullopt>::Foo  ->  Foo()
  //      Foo::Foo        ->  Foo::Foo()
  //      ::print         ->  ::print()
  //
  // - resolved functions (which always have a scope, either global or a class,
  //   never <nullopt>)
  bool operator<( const ScopableName& other ) const;

  ScopeName scope;
  std::string name;
};
}  // namespace Pol::Bscript::Compiler
