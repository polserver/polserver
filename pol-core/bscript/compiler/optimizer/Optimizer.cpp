#include "Optimizer.h"

#include "compiler/Report.h"
#include "compiler/analyzer/Constants.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/IfThenElseStatement.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/astbuilder/SimpleValueCloner.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/optimizer/BinaryOperatorOptimizer.h"
#include "compiler/optimizer/ConstantValidator.h"
#include "compiler/optimizer/ReferencedFunctionGatherer.h"
#include "compiler/optimizer/UnaryOperatorOptimizer.h"
#include "compiler/optimizer/ValueConsumerOptimizer.h"

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

void Optimizer::visit_binary_operator( BinaryOperator& binary_operator )
{
  visit_children( binary_operator );

  switch ( binary_operator.token_id )
  {
  case TOK_ASSIGN:
    break;
  default:
    optimized_replacement = BinaryOperatorOptimizer( binary_operator, report ).optimize();
    break;
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

void Optimizer::visit_if_then_else_statement( IfThenElseStatement& if_then_else )
{
  visit_children( if_then_else );

  if ( auto else_block = dynamic_cast<Block*>( if_then_else.alternative() ) )
  {
    if ( else_block->children.empty() )
      if_then_else.children.erase( if_then_else.children.begin() + 2 );
  }

  auto& predicate = if_then_else.predicate();
  auto predicate_as_long = dynamic_cast<IntegerValue*>( &predicate );
  if ( predicate_as_long )
  {
    if ( predicate_as_long->value )
    {
      optimized_replacement = if_then_else.take_consequent();
    }
    else
    {
      auto alternative = if_then_else.take_alternative();
      if ( alternative )
      {
        optimized_replacement = std::move( alternative );
      }
      else
      {
        std::vector<std::unique_ptr<Statement>> empty;
        optimized_replacement =
            std::make_unique<Block>( if_then_else.source_location, std::move( empty ) );
      }
    }
  }
}

void Optimizer::visit_unary_operator( UnaryOperator& unary_operator )
{
  visit_children( unary_operator );

  optimized_replacement = UnaryOperatorOptimizer( unary_operator ).optimize();
}

void Optimizer::visit_value_consumer( ValueConsumer& consume_value )
{
  visit_children( consume_value );

  optimized_replacement = ValueConsumerOptimizer().optimize( consume_value );
}

}  // namespace Pol::Bscript::Compiler
