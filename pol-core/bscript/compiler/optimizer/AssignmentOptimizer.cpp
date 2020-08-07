#include "AssignmentOptimizer.h"

#include "compiler/ast/ArrayElementAccess.h"
#include "compiler/ast/ArrayElementIndexes.h"
#include "compiler/ast/AssignSubscript.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/GetMember.h"
#include "compiler/ast/SetMember.h"

namespace Pol::Bscript::Compiler
{
AssignmentOptimizer::AssignmentOptimizer( BinaryOperator& assignment ) : assignment( assignment ) {}

std::unique_ptr<Expression> AssignmentOptimizer::optimize()
{
  assignment.lhs().accept( *this );
  return std::move( optimized_result );
}

void AssignmentOptimizer::visit_children( Node& ) {}

void AssignmentOptimizer::visit_array_element_access( ArrayElementAccess& lhs )
{
  auto entity = lhs.take_entity();
  auto indexes = lhs.take_indexes();
  auto rhs = assignment.take_rhs();

  optimized_result =
      std::make_unique<AssignSubscript>( assignment.source_location, false, std::move( entity ),
                                         std::move( indexes ), std::move( rhs ) );
}

void AssignmentOptimizer::visit_get_member( GetMember& lhs )
{
  auto entity = lhs.take_entity();
  auto rhs = assignment.take_rhs();
  optimized_result = std::make_unique<SetMember>( lhs.source_location, false, std::move( entity ),
                                                  lhs.name, std::move( rhs ), lhs.known_member );
}
}  // namespace Pol::Bscript::Compiler
