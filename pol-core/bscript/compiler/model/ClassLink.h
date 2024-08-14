#pragma once

#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class ClassDeclaration;
class ModuleFunctionDeclaration;
class ScopeName;
class UserFunction;

// An analogous class to FunctionLink, but for classes. Allows a UserFunction to
// link to a ClassDeclaration.
class ClassLink
{
public:
  explicit ClassLink( const SourceLocation&, std::string name );
  ClassLink( const ClassLink& ) = delete;
  ClassLink& operator=( const ClassLink& ) = delete;

  [[nodiscard]] ClassDeclaration* class_declaration() const;

  void link_to( ClassDeclaration* );

  const SourceLocation source_location;

  const std::string name;

private:
  ClassDeclaration* linked_class_declaration;
};

}  // namespace Pol::Bscript::Compiler
