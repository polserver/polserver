#include "Optimizer.h"

#include "clib/logfacility.h"
#include "compiler/Report.h"
#include "compiler/analyzer/Constants.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/astbuilder/SimpleValueCloner.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/optimizer/ConstantValidator.h"
#include "compiler/optimizer/ReferencedFunctionGatherer.h"
#include "compiler/optimizer/UnaryOperatorOptimizer.h"

namespace Pol::Bscript::Compiler
{
Optimizer::Optimizer( Constants& constants, Report& report )
  : constants( constants ),
    report( report )
{
}

void Optimizer::optimize( CompilerWorkspace& workspace )
{
  workspace.accept( *this );

  ReferencedFunctionGatherer gatherer( workspace.module_function_declarations );
  workspace.top_level_statements->accept( gatherer );
  if ( auto program = workspace.program.get() )
  {
    program->accept( gatherer );
  }
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

void Optimizer::visit_const_declaration( ConstDeclaration& constant )
{
  visit_children( constant );
  if ( !ConstantValidator().validate( constant.expression() ) )
  {
    report.error( constant, "Const expression must be optimizable.\n", constant, "\n" );
  }
}

void Optimizer::visit_identifier( Identifier& identifier )
{
  if ( auto constant = constants.find( identifier.name ) )
  {
    SimpleValueCloner cloner( report, identifier.source_location );
    optimized_replacement = cloner.clone( constant->expression() );
  }
}

void Optimizer::visit_unary_operator( UnaryOperator& unary_operator )
{
  visit_children( unary_operator );

  optimized_replacement = UnaryOperatorOptimizer( unary_operator ).optimize();
}

}  // namespace Pol::Bscript::Compiler
