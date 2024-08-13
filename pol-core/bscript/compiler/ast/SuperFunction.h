#pragma once

#include "bscript/compiler/ast/UserFunction.h"

namespace Pol::Bscript::Compiler
{
class ClassDeclaration;

class SuperFunction : public UserFunction
{
public:
  SuperFunction( const SourceLocation&, ClassDeclaration* );

  // Do not override accept, so that the UserFunction accept is used.
  void describe_to( std::string& ) const override;

  ClassDeclaration* class_declaration;
};
}  // namespace Pol::Bscript::Compiler
