#include "FunctionLink.h"

#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
FunctionLink::FunctionLink( const SourceLocation& source_location )
    : source_location( source_location ), linked_function( nullptr )
{
}

Function* FunctionLink::function() const
{
  return linked_function;
}

ModuleFunctionDeclaration* FunctionLink::module_function_declaration() const
{
  return dynamic_cast<ModuleFunctionDeclaration*>( linked_function );
}

UserFunction* FunctionLink::user_function() const
{
  return dynamic_cast<UserFunction*>( linked_function );
}

void FunctionLink::link_to( Function* f )
{
  if ( linked_function )
    source_location.internal_error( "function already linked" );
  linked_function = f;
}

}  // namespace Pol::Bscript::Compiler