#include "FunctionResolver.h"

#include "compiler/Report.h"
#include "compiler/ast/Function.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
FunctionResolver::FunctionResolver( Report& report ) : report( report ) {}

const Function* FunctionResolver::find( const std::string& scoped_name )
{
  auto itr = resolved_functions_by_name.find( scoped_name );
  if ( itr != resolved_functions_by_name.end() )
    return ( *itr ).second;
  else
    return nullptr;
}

void FunctionResolver::register_module_function( ModuleFunctionDeclaration* mf )
{
  resolved_functions_by_name[mf->name] = mf;
  auto scoped_name = mf->module_name + "::" + mf->name;
  resolved_functions_by_name[scoped_name] = mf;
}

}  // namespace Pol::Bscript::Compiler
