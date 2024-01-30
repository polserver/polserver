#ifndef POLSERVER_FILE_H
#define POLSERVER_FILE_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Statement;

class File : public Node
{
public:
  explicit File( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  NodeVector comments;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FILE_H
