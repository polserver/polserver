#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class TypeNode;

class Parameter : public Node
{
public:
  Parameter( const SourceLocation&, std::string name, bool question, bool rest,
             std::unique_ptr<TypeNode> type );
  Parameter( const SourceLocation&, std::string name, bool question, bool rest );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  const bool question;
  const bool rest;
};
}  // namespace Pol::Bscript::Compiler
