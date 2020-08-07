#ifndef POLSERVER_METHODCALLARGUMENTLIST_H
#define POLSERVER_METHODCALLARGUMENTLIST_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class MethodCallArgumentList : public Node
{
public:
  MethodCallArgumentList( const SourceLocation& source_location,
                          std::vector<std::unique_ptr<Expression>> arguments );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_METHODCALLARGUMENTLIST_H
