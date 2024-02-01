#include "UninitializedValue.h"


#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UninitializedValue::UninitializedValue( const SourceLocation& source_location )
    : Value( source_location )
{
}

void UninitializedValue::accept( NodeVisitor& visitor )
{
  visitor.visit_uninitialized_value( *this );
}

void UninitializedValue::describe_to( std::string& w ) const
{
  w += "uninitialized-value";
}

}  // namespace Pol::Bscript::Compiler
