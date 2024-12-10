#pragma once

#include "bscript/compiler/ast/types/CallableTypeBinding.h"
#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class ParameterList;
class TypeNode;
class TypeParameterList;

class CallSignature : public MemberSignature, public CallableTypeBinding
{
public:
  CallSignature( const SourceLocation&, std::unique_ptr<TypeParameterList> type_parameters,
                 std::unique_ptr<ParameterList> parameters,
                 std::unique_ptr<TypeNode> type_annotation );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
