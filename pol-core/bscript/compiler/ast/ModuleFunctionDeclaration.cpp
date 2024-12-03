#include "ModuleFunctionDeclaration.h"


#include <utility>

#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"

namespace Pol::Bscript::Compiler
{
ModuleFunctionDeclaration::ModuleFunctionDeclaration(
    const SourceLocation& source_location, std::string module_name, std::string name,
    std::unique_ptr<TypeParameterList> type_params,
    std::unique_ptr<FunctionParameterList> parameter_list,
    std::unique_ptr<TypeNode> type_annotation )
    : Function( source_location, std::move( module_name ), std::move( name ),
                std::move( type_params ), std::move( parameter_list ),
                std::move( type_annotation ) )
{
}

void ModuleFunctionDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_module_function_declaration( *this );
}

void ModuleFunctionDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "module-function-declaration({}::{})", scope, name );
}

}  // namespace Pol::Bscript::Compiler
