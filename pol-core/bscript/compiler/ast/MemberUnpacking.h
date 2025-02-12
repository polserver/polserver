#pragma once

#include <optional>

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class NodeVisitor;
class Variable;

class MemberUnpacking : public Node
{
public:
  // A member unpacking, eg. declaring `y`:
  //
  // {y}
  // {x: y}
  // {["x"]: y}
  MemberUnpacking( const SourceLocation&, std::string scope, std::string name,
                   std::unique_ptr<Expression> member );

  // A rest unpacking, eg:
  //
  // {rest...}
  MemberUnpacking( const SourceLocation&, std::string scope, std::string name );

  // A nested (index- or member-) unpacking, eg:
  //
  // { x: { ... } }
  // { x: [ ... ] }
  // { ["x"]: { ... } }
  // { ["x"]: [ ... ] }
  MemberUnpacking( const SourceLocation&, std::unique_ptr<Expression> member,
                   std::unique_ptr<Node> unpacking );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  // FIXME: Something is telling me that implementing the AST with an optional node means the AST
  // structure is incorrect. Nested unpackings do not have a scoped name, since they do not actually
  // declare a variable.
  //
  // Maybe these different types should be separate AST nodes? TBD.
  const std::optional<ScopableName> scoped_name;
  const bool rest;
  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler
