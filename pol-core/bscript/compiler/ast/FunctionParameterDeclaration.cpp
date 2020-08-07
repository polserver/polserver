#include "FunctionParameterDeclaration.h"

#include "../clib/logfacility.h"
#include <utility>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
FunctionParameterDeclaration::FunctionParameterDeclaration( const SourceLocation& source_location,
                                                            std::string name, bool byref,
                                                            bool unused )
    : Node( source_location ), name( std::move( name ) ), byref( byref ), unused( unused )
{
}

void FunctionParameterDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_function_parameter_declaration( *this );
}

void FunctionParameterDeclaration::describe_to( fmt::Writer& w ) const
{
  w << "function-parameter-declaration(" << name;
  if ( byref )
    w << ", byref";
  if ( unused )
    w << ", unused";
  w << ")";
}

}  // namespace Pol::Bscript::Compiler
