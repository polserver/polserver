#include "Argument.h"


#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Argument::Argument( const SourceLocation& source_location, std::string identifier,
                    std::unique_ptr<Expression> expression )
    : Node( source_location, std::move( expression ) ), identifier( std::move( identifier ) )
{
}

void Argument::accept( NodeVisitor& visitor )
{
  visitor.visit_argument( *this );
}

void Argument::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "argument({})", identifier );
}

std::unique_ptr<Expression> Argument::take_expression()
{
  return take_child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
