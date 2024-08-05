#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class UserFunction;
class FunctionReferenceDescriptor;

class FunctionReferenceRegistrar
{
public:
  FunctionReferenceRegistrar();


  std::vector<FunctionReferenceDescriptor> take_descriptors();

  void lookup_or_register_reference( const UserFunction& node, unsigned& reference_index );

private:
  struct EmittedReference
  {
    explicit EmittedReference( unsigned index, const UserFunction& );
    ~EmittedReference();

    const unsigned index;
    const UserFunction& user_function;
  };

  void register_function_reference( const UserFunction& );

  std::map<std::string, std::unique_ptr<EmittedReference>> registered_references;
  std::vector<std::string> reference_names_in_order;
};

}  // namespace Pol::Bscript::Compiler
