#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class MemberSignature : public Node
{
public:
  MemberSignature( const SourceLocation& );
  MemberSignature( const SourceLocation&, NodeVector children );
  MemberSignature( const SourceLocation&, std::unique_ptr<Node> child );

  virtual void accept( NodeVisitor& ) override;
  virtual void describe_to( std::string& ) const override = 0;
};
}  // namespace Pol::Bscript::Compiler
