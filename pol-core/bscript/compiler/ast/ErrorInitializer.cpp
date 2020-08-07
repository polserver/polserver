#include "ErrorInitializer.h"

#include <format/format.h>
#include <utility>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ErrorInitializer::ErrorInitializer( const SourceLocation& source_location,
                                    std::vector<std::string> names,
                                    std::vector<std::unique_ptr<Expression>> expressions )
    : Expression( source_location, std::move( expressions ) ), names( std::move( names ) )
{
}

void ErrorInitializer::accept( NodeVisitor& visitor )
{
  visitor.visit_error_initializer( *this );
}

void ErrorInitializer::describe_to( fmt::Writer& w ) const
{
  w << "error-initializer";
}

}  // namespace Pol::Bscript::Compiler
