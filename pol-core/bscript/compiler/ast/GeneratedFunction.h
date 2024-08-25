#pragma once

#include "bscript/compiler/ast/UserFunction.h"

namespace Pol::Bscript::Compiler
{
class ClassDeclaration;

class GeneratedFunction : public UserFunction
{
public:
  GeneratedFunction( const SourceLocation&, ClassDeclaration*, UserFunctionType,
                     const std::string& name );

  // Do not override accept, so that the UserFunction accept is used.
  void describe_to( std::string& ) const override;

  ClassDeclaration* class_declaration() const;
};
}  // namespace Pol::Bscript::Compiler
