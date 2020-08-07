#include "FunctionReference.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
FunctionReference::FunctionReference( const SourceLocation& source_location, std::string name,
                                      std::shared_ptr<FunctionLink> function_link )
    : Value( source_location ), name( name ), function_link( std::move( function_link ) )
{
}

void FunctionReference::accept( NodeVisitor& visitor )
{
  visitor.visit_function_reference( *this );
}

void FunctionReference::describe_to( fmt::Writer& w ) const
{
  w << "function-reference(@" << name << ")";
}

}  // namespace Pol::Bscript::Compiler
