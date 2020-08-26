#include "Optimizer.h"

#include "clib/logfacility.h"
#include "compiler/Report.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/optimizer/ReferencedFunctionGatherer.h"
#include "compiler/optimizer/UnaryOperatorOptimizer.h"

namespace Pol::Bscript::Compiler
{
Optimizer::Optimizer( Report& report )
  : report( report )
{
}

void Optimizer::optimize( CompilerWorkspace& workspace )
{
  ReferencedFunctionGatherer gatherer( workspace.module_function_declarations );
  workspace.top_level_statements->accept( gatherer );
  workspace.referenced_module_function_declarations =
      gatherer.take_referenced_module_function_declarations();
}

void Optimizer::visit_children( Node& node )
{
  unsigned i = 0;
  for ( auto& child : node.children )
  {
    child->accept( *this );

    if ( optimized_replacement )
    {
      node.children[i] = std::move( optimized_replacement );
    }

    ++i;
  }
}

void Optimizer::visit_unary_operator( UnaryOperator& unary_operator )
{
  visit_children( unary_operator );

  optimized_replacement = UnaryOperatorOptimizer( unary_operator ).optimize();
}

}  // namespace Pol::Bscript::Compiler
