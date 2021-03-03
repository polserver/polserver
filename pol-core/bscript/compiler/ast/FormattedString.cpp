#include "FormattedString.h"

#include <format/format.h>

#include "clib/strutil.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/ast/StringValue.h"

namespace Pol::Bscript::Compiler
{
FormattedString::FormattedString( const SourceLocation& source_location,
                                  std::unique_ptr<Expression> expression,
                                  std::unique_ptr<StringValue> format )
    : Expression( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( expression ) );
  children.push_back( std::move( format ) );
}

void FormattedString::accept( NodeVisitor& visitor )
{
  visitor.visit_formatted_string( *this );
}

void FormattedString::describe_to( fmt::Writer& w ) const
{
  w << "format-as-string";
}

}  // namespace Pol::Bscript::Compiler