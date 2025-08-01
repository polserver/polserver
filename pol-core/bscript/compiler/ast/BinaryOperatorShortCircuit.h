#pragma once

#include "bscript/compiler/ast/Expression.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class BinaryOperatorShortCircuit;

enum class ShortCircuitOp
{
  AND,
  OR
};

struct JmpLocationLink : public std::enable_shared_from_this<JmpLocationLink>
{
  JmpLocationLink( std::shared_ptr<FlowControlLabel> label, ShortCircuitOp op )
      : jmp_label( std::move( label ) ), oper( op ){};
  void update( const JmpLocationLink& op )
  {
    jmp_label = op.jmp_label;
    oper = op.oper;
  }
  std::shared_ptr<FlowControlLabel> jmp_label;
  ShortCircuitOp oper;
};

class BinaryOperatorShortCircuit : public Expression
{
public:
  BinaryOperatorShortCircuit( const SourceLocation&, std::unique_ptr<Expression> lhs,
                              ShortCircuitOp, std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  Expression& lhs();
  Expression& rhs();
  const ShortCircuitOp oper;
  const std::shared_ptr<FlowControlLabel> end_label;
  // will be set and changed by the ShortCircuitCombiner
  std::shared_ptr<JmpLocationLink> linked_jmp_label;
  bool generate_logical_convert;
};

}  // namespace Pol::Bscript::Compiler
