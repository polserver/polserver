#ifndef POLSERVER_DISAMBIGUATOR_H
#define POLSERVER_DISAMBIGUATOR_H

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Constants;
class CompilerWorkspace;
class Report;

class Disambiguator : public NodeVisitor
{
public:
  Disambiguator( Constants&, Report& );

  void disambiguate( CompilerWorkspace& ast );

  void visit_case_dispatch_group( CaseDispatchGroup& dispatch_group ) override;
  void visit_case_dispatch_groups( CaseDispatchGroups& dispatch_groups ) override;

private:
  Constants& constants;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DISAMBIGUATOR_H
