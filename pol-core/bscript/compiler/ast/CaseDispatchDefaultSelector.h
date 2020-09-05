#ifndef POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H
#define POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchDefaultSelector : public Node
{
public:
  explicit CaseDispatchDefaultSelector( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H
