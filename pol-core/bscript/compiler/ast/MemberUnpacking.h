#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class NodeVisitor;

class MemberUnpacking : public Node
{
public:
  MemberUnpacking( const SourceLocation&, std::string name, std::unique_ptr<Expression> member );
  MemberUnpacking( const SourceLocation&, std::string name );
  MemberUnpacking( const SourceLocation&, std::unique_ptr<Expression> member,
                   std::unique_ptr<Node> unpacking );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  bool rest;
};

}  // namespace Pol::Bscript::Compiler
