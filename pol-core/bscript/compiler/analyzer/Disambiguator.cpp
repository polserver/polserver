#include "Disambiguator.h"

#include "compiler/ast/TopLevelStatements.h"
#include "Constants.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/CaseDispatchGroup.h"
#include "compiler/ast/CaseDispatchGroups.h"
#include "compiler/ast/CaseDispatchSelectors.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/LabelableStatement.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/astbuilder/SimpleValueCloner.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
Disambiguator::Disambiguator( Constants& constants, Report& report )
  : constants( constants ), report( report )
{
}

void Disambiguator::disambiguate( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );
  if ( auto& program = workspace.program )
  {
    program->accept( *this );
  }
  for ( auto& user_function : workspace.user_functions )
  {
    user_function->accept( *this );
  }
}


void Disambiguator::visit_case_dispatch_group( CaseDispatchGroup& dispatch_group )
{
  visit_children( dispatch_group );

  // It is possible that the last CASE-label is actually supposed
  // to be attached to the first statement in the block.
  auto& selectors = dispatch_group.selectors();
  auto& last_selector = selectors.children.back();
  auto last_selector_as_identifier = dynamic_cast<Identifier*>( last_selector.get() );
  if ( last_selector_as_identifier )
  {
    auto& block = dispatch_group.block();
    if ( !block.children.empty() )
    {
      if ( auto labelable = dynamic_cast<LabelableStatement*>( block.children.at( 0 ).get() ) )
      {
        // This is how we tell the last selector should be a label:
        //    - the first statement in the block is a LabelableStatement
        //    - the last selector is a non-constant identifier
        //    - the Optimizer already optimized any constant (Identifier) to its resolved value.
        labelable->relabel( last_selector_as_identifier->name );
        selectors.children.pop_back();
      }
    }
  }
}

void Disambiguator::visit_case_dispatch_groups( CaseDispatchGroups& dispatch_groups )
{
  visit_children( dispatch_groups );

  for ( auto i = 0u; i < dispatch_groups.children.size(); ++i )
  {
    auto& dispatch_group = dispatch_groups.child<CaseDispatchGroup>( i );
    auto& block = dispatch_group.block();
    for ( auto j = 0u; j < block.children.size(); ++j )
    {
      auto labelable_statement = dynamic_cast<LabelableStatement*>( block.children.at( j ).get() );
      if ( labelable_statement )
      {
        if ( auto constant = constants.find( labelable_statement->label ) )
        {
          // we have a case dispatch group where the label is associated with a statement
          // in the block.
          // Example:
          // const FIVE := 5;
          // case (a)
          //    4:
          //        print("4")
          //    FIVE:                   interpreted as:
          //        while(1)                FIVE: while(1)
          //            print("5");
          //            break;
          //        endwhile
          //    6:
          //        print("6");
          // endcase
          //
          // we have this:
          //    dispatch group
          //        selectors: 4
          //        block:
          //          print("4");
          //          FIVE: while(1) ... endwhile
          //    dispatch group:
          //        selectors: 6
          //        block:
          //          print(6)
          std::vector<std::unique_ptr<Statement>> new_block_statements;
          for ( auto k = block.children.begin() + j; k < block.children.end(); ++k )
          {
            new_block_statements.push_back(
                static_unique_pointer_cast<Statement>( std::move( *k ) ) );
          }
          block.children.erase( block.children.begin() + j, block.children.end() );
          std::string label = labelable_statement->label;
          labelable_statement->relabel( "" );
          NodeVector selector_identifiers;
          SimpleValueCloner cloner( report, constant->source_location );
          selector_identifiers.push_back( cloner.clone( constant->expression() ) );

          auto new_selectors = std::make_unique<CaseDispatchSelectors>(
              dispatch_groups.source_location, std::move( selector_identifiers ) );
          auto new_block = std::make_unique<Block>( labelable_statement->source_location,
                                                      std::move( new_block_statements ) );
          auto new_dispatch_group = std::make_unique<CaseDispatchGroup>(
              dispatch_groups.source_location, std::move( new_selectors ), std::move( new_block ) );
          dispatch_groups.children.insert( dispatch_groups.children.begin() + i + 1,
                                           std::move( new_dispatch_group ) );
        }
      }
    }
  }
}

}  // namespace Pol::Bscript::Compiler
