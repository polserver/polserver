#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class DictionaryType : public TypeNode
{
public:
  DictionaryType( const SourceLocation& );
  DictionaryType( const SourceLocation&, NodeVector members );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
