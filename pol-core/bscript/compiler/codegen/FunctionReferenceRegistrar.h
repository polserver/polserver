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


  std::vector<FunctionReferenceDescriptor> take_descriptors(
      const std::map<std::string, size_t>& class_declaration_indexes );

  void lookup_or_register_reference( const UserFunction& node, FlowControlLabel& label,
                                     unsigned& index );

private:
  struct EmittedReference
  {
    explicit EmittedReference( unsigned index, const UserFunction&, FlowControlLabel& label );
    ~EmittedReference();

    const unsigned index;
    const UserFunction& user_function;
    const FlowControlLabel& label;
  };

  void register_function_reference( const UserFunction&, FlowControlLabel& label );

  std::map<std::string, std::unique_ptr<EmittedReference>> registered_references;
  std::vector<std::string> reference_names_in_order;
};

}  // namespace Pol::Bscript::Compiler
