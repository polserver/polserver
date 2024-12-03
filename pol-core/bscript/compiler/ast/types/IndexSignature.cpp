#include "IndexSignature.h"

#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
IndexSignature::IndexSignature( const SourceLocation& source_location,
                                std::unique_ptr<ParameterList> parameters,
                                std::unique_ptr<TypeNode> type )
    : MemberSignature( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
  children.push_back( std::move( type ) );
}

void IndexSignature::describe_to( std::string& w ) const
{
  w += "index-signature";
}
}  // namespace Pol::Bscript::Compiler
