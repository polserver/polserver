#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Identifier;

class ClassParameterList : public Node
{
public:
  ClassParameterList( const SourceLocation& source_location,
                      std::vector<std::unique_ptr<Identifier>> parameters );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
