#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class NodeVisitor;

class IndexBinding : public Node
{
public:
  IndexBinding( const SourceLocation&, std::string name, bool rest );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  const bool rest;
};

}  // namespace Pol::Bscript::Compiler
