#include "FunctionType.h"

#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"

namespace Pol::Bscript::Compiler
{
FunctionType::FunctionType( const SourceLocation& source_location,
                            std::unique_ptr<TypeParameterList> type_parameters,
                            std::unique_ptr<ParameterList> parameters,
                            std::unique_ptr<TypeNode> type_annotation )
    : TypeNode( source_location ),
      CallableTypeBinding( *this, std::move( type_parameters ), std::move( parameters ),
                           std::move( type_annotation ) )
{
}

void FunctionType::describe_to( std::string& w ) const
{
  w += "function-type";
}
}  // namespace Pol::Bscript::Compiler
