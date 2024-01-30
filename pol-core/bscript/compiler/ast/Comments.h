#ifndef POLSERVER_COMMENTS_H
#define POLSERVER_COMMENTS_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class Comments : public Node
{
public:
  explicit Comments( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMMENTS_H
