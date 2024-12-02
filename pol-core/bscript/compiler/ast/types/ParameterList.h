#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class Parameter;

class ParameterList : public Node
{
public:
  ParameterList( const SourceLocation&, std::vector<std::unique_ptr<Parameter>> parameters );
  ParameterList( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
