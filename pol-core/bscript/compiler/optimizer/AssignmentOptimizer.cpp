#include "AssignmentOptimizer.h"

#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/ElementAccess.h"
#include "compiler/ast/ElementAssignment.h"
#include "compiler/ast/ElementIndexes.h"

namespace Pol::Bscript::Compiler
{
AssignmentOptimizer::AssignmentOptimizer( BinaryOperator& assignment ) : assignment( assignment ) {}

std::unique_ptr<Expression> AssignmentOptimizer::optimize()
{
  assignment.lhs().accept( *this );
  return std::move( optimized_result );
}

void AssignmentOptimizer::visit_children( Node& ) {}

void AssignmentOptimizer::visit_element_access( ElementAccess& lhs )
{
  auto entity = lhs.take_entity();
  auto indexes = lhs.take_indexes();
  auto rhs = assignment.take_rhs();

  optimized_result =
      std::make_unique<ElementAssignment>( assignment.source_location, false, std::move( entity ),
                                           std::move( indexes ), std::move( rhs ) );
}

}  // namespace Pol::Bscript::Compiler
