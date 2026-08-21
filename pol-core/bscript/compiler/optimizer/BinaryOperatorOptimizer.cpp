#include "bscript/compiler/optimizer/BinaryOperatorOptimizer.h"

#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"
#include "bscript/compilercfg.h"

#include "bscript/compiler/optimizer/BinaryOperatorShortCircuitOptimizer.h"
#include "bscript/compiler/optimizer/ConstantFolder.h"
#include "bscript/compiler/optimizer/ShortCircuitCombiner.h"

namespace Pol::Bscript::Compiler
{
BinaryOperatorOptimizer::BinaryOperatorOptimizer( BinaryOperator& op, Report& report )
    : op( op ), report( report )
{
}

std::unique_ptr<Expression> BinaryOperatorOptimizer::optimize()
{
  optimized_result = ConstantFolder::fold( op, report );

  // second parse step
  if ( compilercfg.ShortCircuitEvaluation )
  {
    BinaryOperatorShortCircuitOptimizer shortcircuit{ report };
    if ( !optimized_result )
      op.accept( shortcircuit );
    else
      optimized_result->accept( shortcircuit );
    if ( shortcircuit.optimized_result )
    {
      optimized_result = std::move( shortcircuit.optimized_result );
      // third parse step
      ShortCircuitCombiner combiner{ report };
      optimized_result->accept( combiner );
    }
  }
  return std::move( optimized_result );
}

}  // namespace Pol::Bscript::Compiler
