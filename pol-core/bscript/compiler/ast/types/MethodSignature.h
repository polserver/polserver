#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class ParameterList;
class TypeNode;

class MethodSignature : public MemberSignature
{
public:
  MethodSignature( const SourceLocation&, std::string name, bool question,
                   std::unique_ptr<ParameterList> parameters, std::unique_ptr<TypeNode> type );
  MethodSignature( const SourceLocation&, std::string name, bool question,
                   std::unique_ptr<ParameterList> parameters );

  void describe_to( std::string& ) const override;

  const std::string name;
  const bool question;
};
}  // namespace Pol::Bscript::Compiler
