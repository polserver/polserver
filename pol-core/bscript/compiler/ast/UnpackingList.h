#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class UnpackingList : public Node
{
public:
  UnpackingList( const SourceLocation&, std::vector<std::unique_ptr<Node>> unpackings,
                 bool index_unpacking );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool index_unpacking;
};

}  // namespace Pol::Bscript::Compiler
