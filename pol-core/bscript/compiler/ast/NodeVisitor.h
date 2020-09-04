#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol::Bscript::Compiler
{
class Argument;
class BinaryOperator;
class Block;
class ConstDeclaration;
class ExitStatement;
class FloatValue;
class FunctionBody;
class FunctionCall;
class FunctionParameterDeclaration;
class FunctionParameterList;
class Identifier;
class IfThenElseStatement;
class IntegerValue;
class ModuleFunctionDeclaration;
class Node;
class Program;
class ProgramParameterDeclaration;
class ProgramParameterList;
class ReturnStatement;
class StringValue;
class TopLevelStatements;
class UnaryOperator;
class UserFunction;
class ValueConsumer;
class VarStatement;
class WhileLoop;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit_argument( Argument& );
  virtual void visit_binary_operator( BinaryOperator& );
  virtual void visit_block( Block& );
  virtual void visit_const_declaration( ConstDeclaration& );
  virtual void visit_exit_statement( ExitStatement& );
  virtual void visit_float_value( FloatValue& );
  virtual void visit_function_body( FunctionBody& );
  virtual void visit_function_call( FunctionCall& );
  virtual void visit_function_parameter_declaration( FunctionParameterDeclaration& );
  virtual void visit_function_parameter_list( FunctionParameterList& );
  virtual void visit_identifier( Identifier& );
  virtual void visit_if_then_else_statement( IfThenElseStatement& );
  virtual void visit_integer_value( IntegerValue& );
  virtual void visit_module_function_declaration( ModuleFunctionDeclaration& );
  virtual void visit_program( Program& );
  virtual void visit_program_parameter_declaration( ProgramParameterDeclaration& );
  virtual void visit_program_parameter_list( ProgramParameterList& );
  virtual void visit_return_statement( ReturnStatement& );
  virtual void visit_string_value( StringValue& );
  virtual void visit_top_level_statements( TopLevelStatements& );
  virtual void visit_unary_operator( UnaryOperator& );
  virtual void visit_user_function( UserFunction& );
  virtual void visit_value_consumer( ValueConsumer& );
  virtual void visit_var_statement( VarStatement& );
  virtual void visit_while_loop( WhileLoop& );

  virtual void visit_children( Node& parent );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_NODEVISITOR_H
