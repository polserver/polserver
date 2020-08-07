#ifndef POLSERVER_DISAMBIGUATOR_H
#define POLSERVER_DISAMBIGUATOR_H

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Report;

class Disambiguator : public NodeVisitor
{
public:
  explicit Disambiguator( Report& );

  void disambiguate( CompilerWorkspace& ast );

private:
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DISAMBIGUATOR_H
