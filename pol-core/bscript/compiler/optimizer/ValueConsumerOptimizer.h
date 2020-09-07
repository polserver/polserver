#ifndef POLSERVER_VALUECONSUMEROPTIMIZER_H
#define POLSERVER_VALUECONSUMEROPTIMIZER_H

#include <memory>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Statement;

class ValueConsumerOptimizer : public NodeVisitor
{
public:
  std::unique_ptr<Statement> optimized_result;

  void visit_children( Node& ) override;

  void visit_binary_operator( BinaryOperator& ) override;
  void visit_element_assignment( ElementAssignment& ) override;
  void visit_set_member( SetMember& ) override;

  std::unique_ptr<Statement> optimize( ValueConsumer& );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUECONSUMEROPTIMIZER_H
