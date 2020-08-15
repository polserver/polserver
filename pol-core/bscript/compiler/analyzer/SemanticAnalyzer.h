#ifndef POLSERVER_SEMANTICANALYZER_H
#define POLSERVER_SEMANTICANALYZER_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <memory>

#include "clib/maputil.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Report;

class SemanticAnalyzer : public NodeVisitor
{
public:
  explicit SemanticAnalyzer( Report& );

  ~SemanticAnalyzer() override;

  static void register_const_declarations( CompilerWorkspace& );
  void analyze( CompilerWorkspace& );

private:
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
