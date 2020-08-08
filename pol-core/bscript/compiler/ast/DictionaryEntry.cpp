#include "DictionaryEntry.h"

#include <format/format.h>

#include "Expression.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
DictionaryEntry::DictionaryEntry( const SourceLocation& source_location,
                                  std::unique_ptr<Expression> key,
                                  std::unique_ptr<Expression> value )
    : Node( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( key ) );
  children.push_back( std::move( value ) );
}

void DictionaryEntry::accept( NodeVisitor& visitor )
{
  visitor.visit_dictionary_entry( *this );
}

void DictionaryEntry::describe_to( fmt::Writer& w ) const
{
  w << "dictionary-entry";
}

}  // namespace Pol::Bscript::Compiler
