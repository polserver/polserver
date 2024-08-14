#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionLink;

class ClassParameterDeclaration : public Node
{
public:
  ClassParameterDeclaration( const SourceLocation& source_location, std::string name );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  // Name of the base-class
  const std::string name;

  // Link to the constructor of the base-class
  std::shared_ptr<FunctionLink> constructor_link;
};

}  // namespace Pol::Bscript::Compiler
