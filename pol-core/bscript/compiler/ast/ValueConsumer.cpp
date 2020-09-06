#include "ValueConsumer.h"

#include <format/format.h>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ValueConsumer::ValueConsumer( const SourceLocation& source_location,
                              std::unique_ptr<Statement> child )
    : Statement( source_location, std::move( child ) )
{
}

void ValueConsumer::accept( NodeVisitor& visitor )
{
  visitor.visit_value_consumer( *this );
}

void ValueConsumer::describe_to( fmt::Writer& w ) const
{
  w << "value-consumer";
}

}  // namespace Pol::Bscript::Compiler
