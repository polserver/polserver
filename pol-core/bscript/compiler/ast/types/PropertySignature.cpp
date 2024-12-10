#include "PropertySignature.h"

#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
PropertySignature::PropertySignature( const SourceLocation&, std::string name, bool question,
                                      std::unique_ptr<TypeNode> element_type )
    : MemberSignature( source_location, std::move( element_type ) ),
      name( name ),
      question( question )
{
}

PropertySignature::PropertySignature( const SourceLocation&, std::string name, bool question )
    : MemberSignature( source_location ), name( name ), question( question )
{
}

void PropertySignature::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "property-signature({})",
                  Clib::getencodedquotedstring( name ) );
}
}  // namespace Pol::Bscript::Compiler
