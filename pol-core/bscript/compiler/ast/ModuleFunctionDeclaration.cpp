#include "ModuleFunctionDeclaration.h"


#include <utility>

#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ModuleFunctionDeclaration::ModuleFunctionDeclaration(
    const SourceLocation& source_location, std::string module_name, std::string name,
    std::unique_ptr<FunctionParameterList> parameter_list )
    : Function( source_location, std::move( name ), std::move( parameter_list ) ),
      module_name( std::move( module_name ) )
{
}

void ModuleFunctionDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_module_function_declaration( *this );
}

void ModuleFunctionDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "module-function-declaration({}::{})", module_name,
                  name );
}

}  // namespace Pol::Bscript::Compiler
