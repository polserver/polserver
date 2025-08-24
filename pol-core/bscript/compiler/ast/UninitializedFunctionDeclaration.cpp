#include "UninitializedFunctionDeclaration.h"

#include <utility>

#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UninitializedFunctionDeclaration::UninitializedFunctionDeclaration(
    const SourceLocation& source_location, std::string scope, std::string name,
    std::unique_ptr<FunctionParameterList> parameter_list )
    : Function( source_location, std::move( scope ), std::move( name ),
                std::move( parameter_list ) )
{
}

void UninitializedFunctionDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_uninitialized_function_declaration( *this );
}

void UninitializedFunctionDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "uninitialized-function-declaration({}::{})", scope,
                  name );
}

}  // namespace Pol::Bscript::Compiler
