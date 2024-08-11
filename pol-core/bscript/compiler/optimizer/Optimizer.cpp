#include "Optimizer.h"

#include <utility>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/analyzer/Constants.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/BranchSelector.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IfThenElseStatement.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/astbuilder/SimpleValueCloner.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/optimizer/BinaryOperatorOptimizer.h"
#include "bscript/compiler/optimizer/ConstantValidator.h"
#include "bscript/compiler/optimizer/ReferencedFunctionGatherer.h"
#include "bscript/compiler/optimizer/UnaryOperatorOptimizer.h"
#include "bscript/compiler/optimizer/ValueConsumerOptimizer.h"

namespace Pol::Bscript::Compiler
{
Optimizer::Optimizer( Constants& constants, Report& report )
    : constants( constants ), report( report )
{
}

void Optimizer::optimize( CompilerWorkspace& workspace,
                          UserFunctionInclusion user_function_inclusion )
{
  workspace.accept( *this );

  std::vector<UserFunction*> all_user_functions;
  if ( user_function_inclusion == UserFunctionInclusion::All )
  {
    for ( auto& user_function : workspace.user_functions )
    {
      all_user_functions.push_back( user_function.get() );
    }
  }

  std::vector<UserFunction*> exported_functions;
  for ( auto& user_function : workspace.user_functions )
  {
    if ( user_function->exported )
      exported_functions.push_back( user_function.get() );
  }
  ReferencedFunctionGatherer gatherer( workspace.module_function_declarations,
                                       std::move( workspace.user_functions ) );
  workspace.top_level_statements->accept( gatherer );
  if ( auto program = workspace.program.get() )
  {
    program->accept( gatherer );
  }
  for ( auto uf : exported_functions )
  {
    gatherer.reference( uf );
  }
  if ( user_function_inclusion == UserFunctionInclusion::All )
  {
    for ( auto& uf : all_user_functions )
    {
      gatherer.reference( uf );
    }
  }

  for ( auto& cd : workspace.class_declarations )
  {
    cd->accept( gatherer );
  }

  workspace.referenced_module_function_declarations =
      gatherer.take_referenced_module_function_declarations();

  workspace.user_functions = gatherer.take_referenced_user_functions();
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

  optimized_replacement = BinaryOperatorOptimizer( binary_operator, report ).optimize();
}

void Optimizer::visit_branch_selector( BranchSelector& selector )
{
  visit_children( selector );

  auto predicate = selector.predicate();
  if ( auto unary_operator = dynamic_cast<UnaryOperator*>( predicate ) )
  {
    if ( unary_operator->token_id == TOK_LOG_NOT )
    {
      BranchSelector::BranchType branch_type;
      switch ( selector.branch_type )
      {
      case BranchSelector::IfTrue:
        branch_type = BranchSelector::IfFalse;
        break;
      case BranchSelector::IfFalse:
        branch_type = BranchSelector::IfTrue;
        break;
      default:
        selector.internal_error( "Expected conditional branch with predicate" );
      }
      optimized_replacement = std::make_unique<BranchSelector>(
          selector.source_location, branch_type, unary_operator->take_operand() );
    }
  }
  else if ( auto iv = dynamic_cast<IntegerValue*>( predicate ) )
  {
    BranchSelector::BranchType branch_type;
    switch ( selector.branch_type )
    {
    case BranchSelector::IfTrue:
      branch_type = iv->value ? BranchSelector::Always : BranchSelector::Never;
      break;
    case BranchSelector::IfFalse:
      branch_type = !iv->value ? BranchSelector::Always : BranchSelector::Never;
      break;
    default:
      selector.internal_error( "Expected conditional branch with predicate" );
    }
    optimized_replacement =
        std::make_unique<BranchSelector>( selector.source_location, branch_type );
  }
  else if ( auto bv = dynamic_cast<BooleanValue*>( predicate ) )
  {
    BranchSelector::BranchType branch_type;
    switch ( selector.branch_type )
    {
    case BranchSelector::IfTrue:
      branch_type = bv->value ? BranchSelector::Always : BranchSelector::Never;
      break;
    case BranchSelector::IfFalse:
      branch_type = !bv->value ? BranchSelector::Always : BranchSelector::Never;
      break;
    default:
      selector.internal_error( "Expected conditional branch with predicate" );
    }
    optimized_replacement =
        std::make_unique<BranchSelector>( selector.source_location, branch_type );
  }
  else if ( dynamic_cast<UninitializedValue*>( predicate ) )
  {
    BranchSelector::BranchType branch_type;
    switch ( selector.branch_type )
    {
    case BranchSelector::IfTrue:
      branch_type = BranchSelector::Never;
      break;
    case BranchSelector::IfFalse:
      branch_type = BranchSelector::Always;
      break;
    default:
      selector.internal_error( "Expected conditional branch with predicate" );
    }
    optimized_replacement =
        std::make_unique<BranchSelector>( selector.source_location, branch_type );
  }
}

void Optimizer::visit_const_declaration( ConstDeclaration& constant )
{
  visit_children( constant );
  if ( !ConstantValidator().validate( constant.expression() ) )
  {
    report.error( constant,
                  "Const expression must be optimizable.\n"
                  "{}",
                  constant );
  }
}

void Optimizer::visit_identifier( Identifier& identifier )
{
  // We don't have scoped constants, so only global identifiers can be optimized.
  if ( identifier.scoped_name.scope.global() )
  {
    auto name = identifier.string();
    if ( auto constant = constants.find( name ) )
    {
      SimpleValueCloner cloner( report, identifier.source_location );
      optimized_replacement = cloner.clone( constant->expression() );
    }
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

  auto& branch_selector = if_then_else.branch_selector();
  if ( branch_selector.branch_type == BranchSelector::Never )
  {
    optimized_replacement = if_then_else.take_consequent();
  }
  else if ( branch_selector.branch_type == BranchSelector::Always )
  {
    if ( auto alternative = if_then_else.take_alternative() )
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
