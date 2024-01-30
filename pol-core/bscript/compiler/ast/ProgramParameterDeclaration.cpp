#include "ProgramParameterDeclaration.h"


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
  fmt::format_to( std::back_inserter( w ), "{}({}", type(), name );
  if ( unused )
    w += ", unused";
  w += ")";
}

std::string ProgramParameterDeclaration::type() const
{
  return "program-parameter-declaration";
}

void ProgramParameterDeclaration::describe_to( picojson::object& o ) const {
  o["name"] = picojson::value( name );
}

}  // namespace Pol::Bscript::Compiler
