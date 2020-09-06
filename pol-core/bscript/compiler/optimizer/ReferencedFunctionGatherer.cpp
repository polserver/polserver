#include "ReferencedFunctionGatherer.h"

#include "compiler/ast/FunctionCall.h"
#include "compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
ReferencedFunctionGatherer::ReferencedFunctionGatherer(
    std::vector<std::unique_ptr<ModuleFunctionDeclaration>>& all_module_function_declarations )
{
  for ( auto& mfd : all_module_function_declarations )
  {
    unreferenced_module_function_declarations.insert( mfd.get() );
  }
}

void ReferencedFunctionGatherer::visit_function_call( FunctionCall& fc )
{
  visit_children( fc );

  reference( *fc.function_link );
}

void ReferencedFunctionGatherer::reference( FunctionLink& link )
{
  if ( auto mfd = link.module_function_declaration() )
    reference( mfd );
}

void ReferencedFunctionGatherer::reference( ModuleFunctionDeclaration* mfd )
{
  auto itr = unreferenced_module_function_declarations.find( mfd );
  if ( itr != unreferenced_module_function_declarations.end() )
  {
    referenced_module_function_declarations.push_back( mfd );
    unreferenced_module_function_declarations.erase( itr );
  }
}

std::vector<ModuleFunctionDeclaration*>
ReferencedFunctionGatherer::take_referenced_module_function_declarations()
{
  return std::move( referenced_module_function_declarations );
}

}  // namespace Pol::Bscript::Compiler
