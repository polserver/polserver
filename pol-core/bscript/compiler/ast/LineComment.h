#ifndef POLSERVER_LINECOMMENT_H
#define POLSERVER_LINECOMMENT_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class LineComment : public Node
{
public:
  explicit LineComment( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LINECOMMENT_H
