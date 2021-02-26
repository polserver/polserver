#ifndef POLSERVER_CASEDISPATCHSELECTORS_H
#define POLSERVER_CASEDISPATCHSELECTORS_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchSelectors : public Node
{
public:
  CaseDispatchSelectors( const SourceLocation&, NodeVector );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHSELECTORS_H
