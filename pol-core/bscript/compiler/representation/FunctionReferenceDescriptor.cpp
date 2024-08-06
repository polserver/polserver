#include "FunctionReferenceDescriptor.h"

namespace Pol::Bscript::Compiler
{
FunctionReferenceDescriptor::FunctionReferenceDescriptor( int parameter_count, int capture_count,
                                                          bool is_variadic )
    : parameter_count_( parameter_count ),
      capture_count_( capture_count ),
      is_variadic_( is_variadic )
{
}

int FunctionReferenceDescriptor::parameter_count() const
{
  return parameter_count_;
}

int FunctionReferenceDescriptor::capture_count() const
{
  return capture_count_;
}

bool FunctionReferenceDescriptor::is_variadic() const
{
  return is_variadic_;
}
}  // namespace Pol::Bscript::Compiler
