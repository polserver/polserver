#include "DictionaryType.h"

namespace Pol::Bscript::Compiler
{
DictionaryType::DictionaryType( const SourceLocation& source_location )
    : TypeNode( source_location )
{
}

DictionaryType::DictionaryType( const SourceLocation&, NodeVector members )
    : TypeNode( source_location, std::move( members ) )
{
}

void DictionaryType::describe_to( std::string& w ) const
{
  w += "dictionary-type";
}
}  // namespace Pol::Bscript::Compiler
