#ifndef POLSERVER_CONSTANTVALIDATOR_H
#define POLSERVER_CONSTANTVALIDATOR_H

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ConstantValidator : public NodeVisitor
{
public:
  ConstantValidator();

  bool validate( Node& );

  void visit_float_value( FloatValue& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_string_value( StringValue& ) override;

  void visit_children( Node& parent ) override;

private:
  bool valid;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONSTANTVALIDATOR_H
