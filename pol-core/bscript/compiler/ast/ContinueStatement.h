#ifndef POLSERVER_CONTINUESTATEMENT_H
#define POLSERVER_CONTINUESTATEMENT_H

#include "JumpStatement.h"

namespace Pol::Bscript::Compiler
{
class ContinueStatement : public JumpStatement
{
public:
  ContinueStatement( const SourceLocation&, std::string label );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CONTINUESTATEMENT_H
