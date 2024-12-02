#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class TypeNode : public Node
{
public:
  TypeNode( const SourceLocation& );
  TypeNode( const SourceLocation&, NodeVector children );
  TypeNode( const SourceLocation&, std::unique_ptr<Node> child );

  virtual void accept( NodeVisitor& ) override;
  virtual void describe_to( std::string& ) const override = 0;
};
}  // namespace Pol::Bscript::Compiler
