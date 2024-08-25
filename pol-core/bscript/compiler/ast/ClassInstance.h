#pragma once

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class ClassDeclaration;
class NodeVisitor;

class ClassInstance : public Expression
{
public:
  ClassInstance( const SourceLocation&, ClassDeclaration* );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const ClassDeclaration* class_declaration;
};

}  // namespace Pol::Bscript::Compiler
