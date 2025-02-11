#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class NodeVisitor;

class MemberBinding : public Node
{
public:
  MemberBinding( const SourceLocation&, std::string name, std::unique_ptr<Expression> member );
  MemberBinding( const SourceLocation&, std::string name );
  MemberBinding( const SourceLocation&, std::unique_ptr<Expression> member,
                 std::unique_ptr<Node> binding );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  bool rest;
};

}  // namespace Pol::Bscript::Compiler
