#include "ProgramParameterList.h"

#include <format/format.h>

#include "ProgramParameterDeclaration.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ProgramParameterList::ProgramParameterList( const SourceLocation& source_location,
                              std::vector<std::unique_ptr<ProgramParameterDeclaration>> parameters )
    : Node( source_location, std::move( parameters ) )
{
}

void ProgramParameterList::accept( NodeVisitor& visitor )
{
  return visitor.visit_program_parameter_list( *this );
}

void ProgramParameterList::describe_to( fmt::Writer& w ) const
{
  w << "program-parameter-list";
}

}  // namespace Pol::Bscript::Compiler
