#ifndef POLSERVER_SEMANTICANALYZER_H
#define POLSERVER_SEMANTICANALYZER_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <memory>

#include "clib/maputil.h"
#include "compiler/analyzer/FlowControlScopes.h"
#include "compiler/analyzer/LocalVariableScopes.h"
#include "compiler/analyzer/Variables.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Constants;
class LoopStatement;
class Report;
class VarStatement;

class SemanticAnalyzer : public NodeVisitor
{
public:
  SemanticAnalyzer( Constants&, Report& );

  ~SemanticAnalyzer() override;

  static void register_const_declarations( CompilerWorkspace& );
  void analyze( CompilerWorkspace& );

  void visit_block( Block& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_loop_statement( LoopStatement& );
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_user_function( UserFunction& ) override;
  void visit_var_statement( VarStatement& ) override;
  void visit_while_loop( WhileLoop& ) override;

private:
  Constants& constants;
  Report& report;

  Variables globals;
  Variables locals;
  FlowControlScopes break_scopes;
  FlowControlScopes continue_scopes;
  LocalVariableScopes local_scopes;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
