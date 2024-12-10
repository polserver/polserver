#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class TypeParameter;

class TypeParameterList : public Node
{
public:
  TypeParameterList( const SourceLocation&,
                     std::vector<std::unique_ptr<TypeParameter>> type_parameters );
  TypeParameterList( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
