#include "ReturnStatement.h"

#include <format/format.h>

#include "Expression.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ReturnStatement::ReturnStatement( const SourceLocation& source_location,
                                  std::unique_ptr<Expression> expression )
    : Statement( source_location, std::move( expression ) )
{
}

void ReturnStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_return_statement( *this );
}

void ReturnStatement::describe_to( fmt::Writer& w ) const
{
  w << "return-statement";
}

}  // namespace Pol::Bscript::Compiler
