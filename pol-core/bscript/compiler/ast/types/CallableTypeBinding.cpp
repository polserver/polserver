#include "CallableTypeBinding.h"

#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"

namespace Pol::Bscript::Compiler
{
CallableTypeBinding::CallableTypeBinding( Node& node,
                                          std::unique_ptr<TypeParameterList> type_parameters,
                                          std::unique_ptr<ParameterList> parameters,
                                          std::unique_ptr<TypeNode> type_annotation )
    : node( node )
{
  auto& children = node.children;
  if ( type_annotation )
  {
    children.reserve( 3 );
    children.push_back( std::move( type_parameters ) );
    children.push_back( std::move( parameters ) );
    children.push_back( std::move( type_annotation ) );
  }
  else
  {
    children.reserve( 2 );
    children.push_back( std::move( type_parameters ) );
    children.push_back( std::move( parameters ) );
  }
}

ParameterList& CallableTypeBinding::parameters()
{
  return node.child<ParameterList>( 1 );
}

TypeNode* CallableTypeBinding::type_annotation()
{
  if ( node.children.size() == 3 )
  {
    return &node.child<TypeNode>( 2 );
  }

  return nullptr;
}

TypeParameterList& CallableTypeBinding::type_parameters()
{
  return node.child<TypeParameterList>( 0 );
}
}  // namespace Pol::Bscript::Compiler
