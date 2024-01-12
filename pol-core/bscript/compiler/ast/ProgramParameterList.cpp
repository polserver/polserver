#include "ProgramParameterList.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"

namespace Pol::Bscript::Compiler
{
ProgramParameterList::ProgramParameterList(
    const SourceLocation& source_location,
    std::vector<std::unique_ptr<ProgramParameterDeclaration>> parameters )
    : Node( source_location, std::move( parameters ) )
{
}

void ProgramParameterList::accept( NodeVisitor& visitor )
{
  return visitor.visit_program_parameter_list( *this );
}

void ProgramParameterList::describe_to( std::string& w ) const
{
  w += "program-parameter-list";
}

}  // namespace Pol::Bscript::Compiler
