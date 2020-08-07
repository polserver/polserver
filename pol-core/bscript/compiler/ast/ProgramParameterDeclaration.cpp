#include "ProgramParameterDeclaration.h"

#include "../clib/logfacility.h"
#include <boost/make_unique.hpp>
#include <format/format.h>
#include <utility>

#include "ArrayInitializer.h"
#include "ConstDeclaration.h"
#include "ErrorInitializer.h"
#include "Expression.h"
#include "FloatValue.h"
#include "Identifier.h"
#include "IntegerValue.h"
#include "NodeVisitor.h"
#include "StringValue.h"
#include "UninitializedValue.h"

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

void ProgramParameterDeclaration::describe_to( fmt::Writer& w ) const
{
  w << "program-parameter-declaration(" << name;
  if ( unused )
    w << ", unused";
  w << ")";
}

}  // namespace Pol::Bscript::Compiler
