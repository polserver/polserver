#include "FunctionParameterDeclaration.h"

#include <utility>

#include "clib/logfacility.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
FunctionParameterDeclaration::FunctionParameterDeclaration(
    const SourceLocation& source_location, std::string name, bool byref, bool unused,
    std::unique_ptr<Expression> default_value )
  : Node( source_location, std::move( default_value ) ),
    name( std::move( name ) ),
    byref( byref ),
    unused( unused )
{
}

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

Expression* FunctionParameterDeclaration::default_value()
{
  return children.empty() ? nullptr : &child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
