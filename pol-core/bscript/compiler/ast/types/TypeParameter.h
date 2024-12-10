#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class TypeNode;
class TypeConstraint;

class TypeParameter : public Node
{
public:
  TypeParameter( const SourceLocation&, std::string name );
  TypeParameter( const SourceLocation&, std::string name, std::unique_ptr<TypeNode> default_value );
  TypeParameter( const SourceLocation&, std::string name,
                 std::unique_ptr<TypeConstraint> type_constraint );
  TypeParameter( const SourceLocation&, std::string name, std::unique_ptr<TypeNode> default_value,
                 std::unique_ptr<TypeConstraint> type_constraint );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  TypeNode* default_value();
  TypeConstraint* type_constraint();
};
}  // namespace Pol::Bscript::Compiler
