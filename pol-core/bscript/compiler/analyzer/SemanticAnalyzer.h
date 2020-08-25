#ifndef POLSERVER_SEMANTICANALYZER_H
#define POLSERVER_SEMANTICANALYZER_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <memory>

#include "clib/maputil.h"
#include "compiler/analyzer/Variables.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Identifier;
class Report;
class VarStatement;

class SemanticAnalyzer : public NodeVisitor
{
public:
  explicit SemanticAnalyzer( Report& );

  ~SemanticAnalyzer() override;

  static void register_const_declarations( CompilerWorkspace& );
  void analyze( CompilerWorkspace& );

  void visit_identifier( Identifier& ) override;
  void visit_var_statement( VarStatement& ) override;

private:
  Report& report;

  Variables globals;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
