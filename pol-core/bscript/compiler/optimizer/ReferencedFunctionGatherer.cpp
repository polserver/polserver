#include "ReferencedFunctionGatherer.h"

#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
ReferencedFunctionGatherer::ReferencedFunctionGatherer(
    std::vector<std::unique_ptr<ModuleFunctionDeclaration>>& all_module_function_declarations,
    std::vector<std::unique_ptr<UserFunction>> all_user_functions )
{
  for ( auto& mfd : all_module_function_declarations )
  {
    unreferenced_module_function_declarations.insert( mfd.get() );
  }
  for ( auto& uf : all_user_functions )
  {
    UserFunction* p = uf.get();
    unreferenced_user_functions[p] = std::move( uf );
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
  else if ( auto uf = link.user_function() )
    reference( uf );
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

void ReferencedFunctionGatherer::reference( UserFunction* uf )
{
  auto itr = unreferenced_user_functions.find( uf );
  if ( itr != unreferenced_user_functions.end() )
  {
    referenced_user_functions.push_back( std::move( ( *itr ).second ) );
    unreferenced_user_functions.erase( itr );
    uf->accept( *this );
  }
}

std::vector<std::unique_ptr<UserFunction>>
ReferencedFunctionGatherer::take_referenced_user_functions()
{
  return std::move( referenced_user_functions );
}

std::vector<ModuleFunctionDeclaration*>
ReferencedFunctionGatherer::take_referenced_module_function_declarations()
{
  return std::move( referenced_module_function_declarations );
}

}  // namespace Pol::Bscript::Compiler
