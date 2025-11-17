#pragma once

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class RegularExpressionValue : public Value
{
public:
  RegularExpressionValue( const SourceLocation& source_location, std::string pattern,
                          std::string flags );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  const std::string pattern;
  const std::string flags;
};

}  // namespace Pol::Bscript::Compiler
