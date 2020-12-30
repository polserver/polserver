#ifndef POLSERVER_CASEDISPATCHGROUPS_H
#define POLSERVER_CASEDISPATCHGROUPS_H

#include "bscript/compiler/ast/Node.h"

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

  CaseDispatchGroup& dispatch_group( size_t );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHGROUPS_H
