#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Node;
class ParameterList;
class TypeNode;
class TypeParameterList;

class CallableTypeBinding  //: public Node
{
public:
  CallableTypeBinding( Node&, std::unique_ptr<TypeParameterList> type_parameters,
                       std::unique_ptr<ParameterList> parameters,
                       std::unique_ptr<TypeNode> type_annotation );

  ParameterList& parameters();
  TypeParameterList& type_parameters();
  TypeNode* type_annotation();

  Node& node;
};
}  // namespace Pol::Bscript::Compiler
