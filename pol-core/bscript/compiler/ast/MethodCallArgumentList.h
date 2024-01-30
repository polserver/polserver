#ifndef POLSERVER_METHODCALLARGUMENTLIST_H
#define POLSERVER_METHODCALLARGUMENTLIST_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class MethodCallArgumentList : public Node
{
public:
  MethodCallArgumentList( const SourceLocation&,
                          std::vector<std::unique_ptr<Expression>> arguments );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_METHODCALLARGUMENTLIST_H
