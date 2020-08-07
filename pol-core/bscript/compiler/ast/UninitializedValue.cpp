#include "UninitializedValue.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UninitializedValue::UninitializedValue( const SourceLocation& source_location )
    : Expression( source_location )
{
}

void UninitializedValue::accept( NodeVisitor& visitor )
{
  visitor.visit_uninitialized_value( *this );
}

void UninitializedValue::describe_to( fmt::Writer& w ) const
{
  w << "uninitialized-value";
}

}  // namespace Pol::Bscript::Compiler
