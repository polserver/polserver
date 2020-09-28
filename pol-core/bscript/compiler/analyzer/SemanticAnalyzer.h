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

class SemanticAnalyzer : public NodeVisitor
{
public:
  SemanticAnalyzer( CompilerWorkspace&, Report& );

  ~SemanticAnalyzer() override;

  static void register_const_declarations( CompilerWorkspace&, Report& );
  void analyze();

  void visit_basic_for_loop( BasicForLoop& ) override;
  void visit_block( Block& ) override;
  void visit_case_statement( CaseStatement& ) override;
  void visit_case_dispatch_group( CaseDispatchGroup& ) override;
  void visit_case_dispatch_selectors( CaseDispatchSelectors& ) override;
  void visit_cstyle_for_loop( CstyleForLoop& ) override;
  void visit_do_while_loop( DoWhileLoop& ) override;
  void visit_foreach_loop( ForeachLoop& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_function_reference( FunctionReference& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_jump_statement( JumpStatement& ) override;
  void visit_loop_statement( LoopStatement& );
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& ) override;
  void visit_user_function( UserFunction& ) override;
  void visit_var_statement( VarStatement& ) override;
  void visit_variable_assignment_statement( VariableAssignmentStatement& ) override;
  void visit_while_loop( WhileLoop& ) override;

private:
  bool report_function_name_conflict( const SourceLocation&, const std::string& function_name,
                                      const std::string& element_description );
  static bool report_function_name_conflict( const CompilerWorkspace&, Report&,
                                             const SourceLocation&,
                                             const std::string& function_name,
                                             const std::string& element_description );

  CompilerWorkspace& workspace;
  Report& report;

  Variables globals;
  Variables locals;
  FlowControlScopes break_scopes;
  FlowControlScopes continue_scopes;
  LocalVariableScopes local_scopes;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
