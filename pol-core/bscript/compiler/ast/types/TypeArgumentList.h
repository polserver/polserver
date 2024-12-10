#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class TypeNode;

class TypeArgumentList : public Node
{
public:
  TypeArgumentList( const SourceLocation&, std::vector<std::unique_ptr<TypeNode>> type_arguments );
  TypeArgumentList( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
