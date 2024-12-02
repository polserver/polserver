#include "Parameter.h"

#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
Parameter::Parameter( const SourceLocation& source_location, std::string name, bool question,
                      bool rest, std::unique_ptr<TypeNode> type )
    : Node( source_location, std::move( type ) ),
      name( std::move( name ) ),
      question( question ),
      rest( rest )
{
}

Parameter::Parameter( const SourceLocation& source_location, std::string name, bool question,
                      bool rest )
    : Node( source_location ), name( std::move( name ) ), question( question ), rest( rest )
{
}

void Parameter::accept( NodeVisitor& ) {}

void Parameter::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "parameter({}", name );
  if ( rest )
    w += "...";
  else if ( question )
    w += "?";
  w += ")";
}
}  // namespace Pol::Bscript::Compiler
