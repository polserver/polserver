#include "FunctionParameterList.h"

#include <format/format.h>

#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
FunctionParameterList::FunctionParameterList(
    const SourceLocation& source_location,
    std::vector<std::unique_ptr<FunctionParameterDeclaration>> parameters )
    : Node( source_location, std::move( parameters ) )
{
}

void FunctionParameterList::accept( NodeVisitor& visitor )
{
  return visitor.visit_function_parameter_list( *this );
}

void FunctionParameterList::describe_to( std::string& w ) const
{
  w += "function-parameter-list";
}

}  // namespace Pol::Bscript::Compiler
