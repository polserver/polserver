#include "ErrorInitializer.h"


#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"

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

void ErrorInitializer::describe_to( std::string& w ) const
{
  w += type();
}

std::string ErrorInitializer::type() const
{
  return "error-initializer";
}

}  // namespace Pol::Bscript::Compiler
