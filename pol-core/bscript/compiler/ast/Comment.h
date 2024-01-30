#ifndef POLSERVER_COMMENT_H
#define POLSERVER_COMMENT_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class Comment : public Node
{
public:
  explicit Comment( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMMENT_H
