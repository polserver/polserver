#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Argument;
class FloatValue;
class FunctionCall;
class FunctionParameterDeclaration;
class FunctionParameterList;
class ModuleFunctionDeclaration;
class Node;
class StringValue;
class TopLevelStatements;
class ValueConsumer;
class VarStatement;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_argument( Argument& );
  virtual void visit_float_value( FloatValue& );
  virtual void visit_function_call( FunctionCall& );
  virtual void visit_function_parameter_declaration( FunctionParameterDeclaration& );
  virtual void visit_function_parameter_list( FunctionParameterList& );
  virtual void visit_module_function_declaration( ModuleFunctionDeclaration& );
  virtual void visit_string_value( StringValue& );
  virtual void visit_top_level_statements( TopLevelStatements& );
  virtual void visit_value_consumer( ValueConsumer& );
  virtual void visit_var_statement( VarStatement& );

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_NODEVISITOR_H
