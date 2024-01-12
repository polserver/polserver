#include "ProgramParameterDeclaration.h"

#include <format/format.h>
#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ProgramParameterDeclaration::ProgramParameterDeclaration( const SourceLocation& source_location,
                                                          std::string name, bool unused )
    : Node( source_location ), name( std::move( name ) ), unused( unused )
{
}
void ProgramParameterDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_program_parameter_declaration( *this );
}

void ProgramParameterDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "program-parameter-declaration({}", name );
  if ( unused )
    w += ", unused";
  w += ")";
}

}  // namespace Pol::Bscript::Compiler
