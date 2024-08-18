#include "Argument.h"


#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Argument::Argument( const SourceLocation& source_location, const ScopableName& identifier,
                    std::unique_ptr<Expression> expression, bool spread )
    : Node( source_location, std::move( expression ) ),
      identifier( std::make_unique<ScopableName>( identifier ) ),
      spread( spread )
{
}

Argument::Argument( const SourceLocation& source_location, std::unique_ptr<Expression> expression,
                    bool spread )
    : Node( source_location, std::move( expression ) ), identifier( nullptr ), spread( spread )
{
}

void Argument::accept( NodeVisitor& visitor )
{
  visitor.visit_argument( *this );
}

void Argument::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "argument({}{})", spread ? "..." : "",
                  identifier ? identifier->string() : "" );
}

std::unique_ptr<Expression> Argument::take_expression()
{
  return take_child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
