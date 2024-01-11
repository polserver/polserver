#ifndef POLSERVER_CONSTANTVALIDATOR_H
#define POLSERVER_CONSTANTVALIDATOR_H

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ConstantValidator : public NodeVisitor
{
public:
  ConstantValidator();

  bool validate( Node& );

  void visit_array_initializer( ArrayInitializer& ) override;
  void visit_dictionary_initializer( DictionaryInitializer& ) override;
  void visit_error_initializer( ErrorInitializer& ) override;
  void visit_float_value( FloatValue& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_struct_initializer( StructInitializer& ) override;
  void visit_uninitialized_value( UninitializedValue& ) override;

  void visit_children( Node& parent ) override;

private:
  bool valid;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONSTANTVALIDATOR_H
