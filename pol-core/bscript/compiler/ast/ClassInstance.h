#pragma once

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class ClassInstance : public Expression
{
public:
  ClassInstance( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
