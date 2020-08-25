#ifndef POLSERVER_INSTRUCTIONGENERATOR_H
#define POLSERVER_INSTRUCTIONGENERATOR_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <string>

namespace Pol::Bscript::Compiler
{
class InstructionEmitter;

class InstructionGenerator : public NodeVisitor
{
public:
  explicit InstructionGenerator( InstructionEmitter& );

  void visit_float_value( FloatValue& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_value_consumer( ValueConsumer& ) override;
  void visit_var_statement( VarStatement& ) override;

private:
  // There are two of these because sometimes when calling a method
  // on InstructionEmitter, the variable name reads better as a noun,
  // and sometimes it reads better as a verb.
  InstructionEmitter& emitter;
  InstructionEmitter& emit;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_INSTRUCTIONGENERATOR_H
