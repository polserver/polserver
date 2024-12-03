#include "CallSignature.h"

#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
CallSignature::CallSignature( const SourceLocation& source_location,
                              std::unique_ptr<ParameterList> parameters,
                              std::unique_ptr<TypeNode> type )
    : MemberSignature( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
  children.push_back( std::move( type ) );
}

CallSignature::CallSignature( const SourceLocation& source_location,
                              std::unique_ptr<ParameterList> parameters )
    : MemberSignature( source_location, std::move( parameters ) )
{
}

void CallSignature::describe_to( std::string& w ) const
{
  w += "call-signature";
}
}  // namespace Pol::Bscript::Compiler
