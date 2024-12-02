#include "FunctionType.h"

#include "bscript/compiler/ast/types/ParameterList.h"

namespace Pol::Bscript::Compiler
{
FunctionType::FunctionType( const SourceLocation& source_location,
                            std::unique_ptr<ParameterList> parameters,
                            std::unique_ptr<TypeNode> type )
    : TypeNode( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
  children.push_back( std::move( type ) );
}

void FunctionType::describe_to( std::string& w ) const
{
  w += "function-type";
}
}  // namespace Pol::Bscript::Compiler
