#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class ClassDeclaration : public Node
{
public:
  // TODO the other nodes
  ClassDeclaration( const SourceLocation& source_location, std::string name );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
};

}  // namespace Pol::Bscript::Compiler
