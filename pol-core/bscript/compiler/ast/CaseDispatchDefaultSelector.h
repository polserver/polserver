#ifndef POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H
#define POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchDefaultSelector : public Node
{
public:
  explicit CaseDispatchDefaultSelector( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEDISPATCHDEFAULTSELECTOR_H
