#include "MethodSignature.h"

#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
MethodSignature::MethodSignature( const SourceLocation& source_location, std::string name,
                                  bool question, std::unique_ptr<ParameterList> parameters,
                                  std::unique_ptr<TypeNode> type )
    : MemberSignature( source_location ), name( name ), question( question )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
  children.push_back( std::move( type ) );
}

MethodSignature::MethodSignature( const SourceLocation&, std::string name, bool question,
                                  std::unique_ptr<ParameterList> parameters )
    : MemberSignature( source_location, std::move( parameters ) ),
      name( name ),
      question( question )
{
}

void MethodSignature::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "method-signature({}{})", name, question ? "?" : "" );
}
}  // namespace Pol::Bscript::Compiler
