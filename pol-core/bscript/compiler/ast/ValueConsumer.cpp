#include "ValueConsumer.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ValueConsumer::ValueConsumer( const SourceLocation& source_location,
                              std::unique_ptr<Expression> child )
    : Expression( source_location, std::move( child ) )
{
}

void ValueConsumer::accept( NodeVisitor& visitor )
{
  visitor.visit_value_consumer( *this );
}

void ValueConsumer::describe_to( std::string& w ) const
{
  w += "value-consumer";
}

}  // namespace Pol::Bscript::Compiler
