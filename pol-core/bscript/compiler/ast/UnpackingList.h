#pragma once

#include "bscript/compiler/ast/Node.h"
#include "clib/rawtypes.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class UnpackingList : public Node
{
public:
  UnpackingList( const SourceLocation&, std::vector<std::unique_ptr<Node>> unpackings,
                 bool index_unpacking );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool index_unpacking;

  // Filled by SemanticAnalyzer. If <128, then a valid value.
  u8 rest_index = 0xFF;
};

}  // namespace Pol::Bscript::Compiler
