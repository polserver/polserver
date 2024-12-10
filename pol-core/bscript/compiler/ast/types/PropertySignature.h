#pragma once

#include "bscript/compiler/ast/types/MemberSignature.h"

namespace Pol::Bscript::Compiler
{
class TypeNode;

class PropertySignature : public MemberSignature
{
public:
  PropertySignature( const SourceLocation&, std::string name, bool question,
                     std::unique_ptr<TypeNode> element_type );
  PropertySignature( const SourceLocation&, std::string name, bool question );

  void describe_to( std::string& ) const override;

  const std::string name;
  const bool question;
};
}  // namespace Pol::Bscript::Compiler
