#pragma once

#include <string>

namespace Pol::Bscript::Compiler
{

class FunctionReferenceDescriptor
{
public:
  FunctionReferenceDescriptor( int parameter_count, int capture_count, bool is_variadic );
  int parameter_count() const;
  int capture_count() const;
  bool is_variadic() const;

private:
  int parameter_count_;
  int capture_count_;
  bool is_variadic_;
};

}  // namespace Pol::Bscript::Compiler
