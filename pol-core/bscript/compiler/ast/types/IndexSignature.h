#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class ParameterList;
class TypeNode;

class IndexSignature : public MemberSignature
{
public:
  IndexSignature( const SourceLocation&, std::unique_ptr<ParameterList> parameters,
                  std::unique_ptr<TypeNode> type );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
