#pragma once

#include "bscript/compiler/ast/UserFunction.h"

namespace Pol::Bscript::Compiler
{
class DefaultConstructorFunction : public UserFunction
{
public:
  DefaultConstructorFunction( const SourceLocation&, const std::string& class_name );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
