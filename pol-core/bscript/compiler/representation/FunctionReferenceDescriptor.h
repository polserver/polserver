#pragma once

#include <string>

namespace Pol::Bscript::Compiler
{

class FunctionReferenceDescriptor
{
public:
  FunctionReferenceDescriptor( unsigned address, int parameter_count, int capture_count,
                               bool is_variadic, unsigned class_index );
  unsigned address() const;
  int parameter_count() const;
  int capture_count() const;
  bool is_variadic() const;
  unsigned class_index() const;

private:
  unsigned address_;
  int parameter_count_;
  int capture_count_;
  bool is_variadic_;
  unsigned class_index_;
};

}  // namespace Pol::Bscript::Compiler
