#include "FunctionReferenceDescriptor.h"

namespace Pol::Bscript::Compiler
{
FunctionReferenceDescriptor::FunctionReferenceDescriptor(
    unsigned address, int parameter_count, int capture_count, bool is_variadic,
    unsigned class_index, bool is_constructor, std::vector<unsigned> default_parameter_addresses )
    : address_( address ),
      parameter_count_( parameter_count ),
      capture_count_( capture_count ),
      is_variadic_( is_variadic ),
      class_index_( class_index ),
      is_constructor_( is_constructor ),
      default_parameter_addresses_( std::move( default_parameter_addresses ) )
{
}

unsigned FunctionReferenceDescriptor::address() const
{
  return address_;
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

unsigned FunctionReferenceDescriptor::class_index() const
{
  return class_index_;
}

bool FunctionReferenceDescriptor::is_constructor() const
{
  return is_constructor_;
}

const std::vector<unsigned>& FunctionReferenceDescriptor::default_parameter_addresses() const
{
  return default_parameter_addresses_;
}
}  // namespace Pol::Bscript::Compiler
