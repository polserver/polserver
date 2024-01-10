#include "MethodCallArgumentList.h"

#include <format/format.h>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
MethodCallArgumentList::MethodCallArgumentList( const SourceLocation& source_location,
                                                std::vector<std::unique_ptr<Expression>> arguments )
    : Node( source_location, std::move( arguments ) )
{
}

void MethodCallArgumentList::accept( NodeVisitor& visitor )
{
  visitor.visit_method_call_argument_list( *this );
}
void MethodCallArgumentList::describe_to( std::string& w ) const
{
  w += "method-call-argument-list";
}

}  // namespace Pol::Bscript::Compiler