#ifndef POLSERVER_CASEDISPATCHGROUPS_H
#define POLSERVER_CASEDISPATCHGROUPS_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchGroup;

class CaseDispatchGroups : public Node
{
public:
  CaseDispatchGroups( const SourceLocation& context,
                      std::vector<std::unique_ptr<CaseDispatchGroup>> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  CaseDispatchGroup& dispatch_group( int );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHGROUPS_H
