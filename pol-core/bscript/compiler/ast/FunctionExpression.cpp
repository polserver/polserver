#include "FunctionExpression.h"

#include <fmt/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
FunctionExpression::FunctionExpression( const SourceLocation& source_location,
                                        std::shared_ptr<FunctionLink> function_link )
    : Value( source_location ), function_link( std::move( function_link ) )
{
}

void FunctionExpression::accept( NodeVisitor& visitor )
{
  visitor.visit_function_expression( *this );
}

void FunctionExpression::describe_to( std::string& w ) const
{
  std::string name;

  if ( auto fn = function_link->function() )
    name = fn->name;
  else
    name = "unknown";

  fmt::format_to( std::back_inserter( w ), "function-expression({})", name );
}

}  // namespace Pol::Bscript::Compiler
