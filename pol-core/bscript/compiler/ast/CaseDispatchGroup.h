#ifndef POLSERVER_CASEDISPATCHGROUP_H
#define POLSERVER_CASEDISPATCHGROUP_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Block;
class CaseDispatchSelectors;
class FlowControlLabel;

class CaseDispatchGroup : public Node
{
public:
  CaseDispatchGroup( const SourceLocation&, std::unique_ptr<CaseDispatchSelectors>,
                     std::unique_ptr<Block> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  CaseDispatchSelectors& selectors();
  Block& block();

  // this is only needed for parity.  I think we can generate valid
  // (and slightly more efficient) code without it.
  const std::shared_ptr<FlowControlLabel> break_label;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHGROUP_H
