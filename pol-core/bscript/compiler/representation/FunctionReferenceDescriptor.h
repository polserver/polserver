#pragma once

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{

class FunctionReferenceDescriptor
{
public:
  FunctionReferenceDescriptor( unsigned address, int parameter_count, int capture_count,
                               bool is_variadic, unsigned class_index, bool is_constructor,
                               std::vector<unsigned> default_parameter_addresses );
  unsigned address() const;
  int parameter_count() const;
  int capture_count() const;
  bool is_variadic() const;
  unsigned class_index() const;
  bool is_constructor() const;
  const std::vector<unsigned>& default_parameter_addresses() const;

private:
  unsigned address_;
  int parameter_count_;
  int capture_count_;
  bool is_variadic_;
  unsigned class_index_;  // Has value for methods and constructors, NOT static functions
  bool is_constructor_;   // Must be `true` to call with MTH_NEW
  std::vector<unsigned> default_parameter_addresses_;
};

}  // namespace Pol::Bscript::Compiler
