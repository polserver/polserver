#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class ParameterList;
class TypeNode;

class CallSignature : public MemberSignature
{
public:
  CallSignature( const SourceLocation&, std::unique_ptr<ParameterList> parameters,
                 std::unique_ptr<TypeNode> type );
  CallSignature( const SourceLocation&, std::unique_ptr<ParameterList> parameters );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
