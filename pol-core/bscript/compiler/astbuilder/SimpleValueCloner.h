#ifndef POLSERVER_SIMPLEVALUECLONER_H
#define POLSERVER_SIMPLEVALUECLONER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <memory>

#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class Report;

class SimpleValueCloner : private NodeVisitor
{
public:
  explicit SimpleValueCloner( Report&, const SourceLocation& );

  std::unique_ptr<Node> clone( Node& );

  void visit_array_initializer( ArrayInitializer& ) override;
  void visit_boolean_value( BooleanValue& ) override;
  void visit_dictionary_initializer( DictionaryInitializer& ) override;
  void visit_error_initializer( ErrorInitializer& ) override;
  void visit_float_value( FloatValue& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_integer_value( IntegerValue& ) override;
  void visit_string_value( StringValue& ) override;
  void visit_struct_initializer( StructInitializer& ) override;
  void visit_uninitialized_value( UninitializedValue& ) override;

  void visit_children( Node& parent ) override;

private:
  Report& report;
  std::unique_ptr<Node> cloned_value;
  SourceLocation use_source_location;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SIMPLEVALUECLONER_H
