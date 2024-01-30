#ifndef POLSERVER_DEBUGSTATEMENTMARKER_H
#define POLSERVER_DEBUGSTATEMENTMARKER_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class DebugStatementMarker : public Statement
{
public:
  DebugStatementMarker( const SourceLocation&, std::string text, unsigned start_index );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  const std::string text;
  const unsigned start_index;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGSTATEMENTMARKER_H
