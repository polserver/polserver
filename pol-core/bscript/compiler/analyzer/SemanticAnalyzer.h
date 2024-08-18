#ifndef POLSERVER_SEMANTICANALYZER_H
#define POLSERVER_SEMANTICANALYZER_H

#include "bscript/compiler/ast/NodeVisitor.h"

#include <map>
#include <memory>
#include <stack>

#include "bscript/compiler/analyzer/FlowControlScopes.h"
#include "bscript/compiler/analyzer/LocalVariableScopes.h"
#include "bscript/compiler/analyzer/Variables.h"
#include "bscript/compiler/model/ScopeName.h"

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
  void visit_class_declaration( ClassDeclaration& ) override;
  void visit_class_parameter_declaration( ClassParameterDeclaration& ) override;
  void visit_case_statement( CaseStatement& ) override;
  void visit_case_dispatch_group( CaseDispatchGroup& ) override;
  void visit_case_dispatch_selectors( CaseDispatchSelectors& ) override;
  void visit_cstyle_for_loop( CstyleForLoop& ) override;
  void visit_do_while_loop( DoWhileLoop& ) override;
  void visit_foreach_loop( ForeachLoop& ) override;
  void visit_function_call( FunctionCall& ) override;
  void visit_function_parameter_list( FunctionParameterList& ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& ) override;
  void visit_function_expression( FunctionExpression& ) override;
  void visit_function_reference( FunctionReference& ) override;
  void visit_identifier( Identifier& ) override;
  void visit_jump_statement( JumpStatement& ) override;
  void visit_loop_statement( LoopStatement& );
  void visit_program( Program& ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& ) override;
  void visit_return_statement( ReturnStatement& ) override;
  void visit_user_function( UserFunction& ) override;
  void visit_var_statement( VarStatement& ) override;
  void visit_variable_assignment_statement( VariableAssignmentStatement& ) override;
  void visit_while_loop( WhileLoop& ) override;

private:
  bool report_function_name_conflict( const SourceLocation&, const std::string& function_name,
                                      const std::string& element_description );
  static bool report_function_name_conflict( const CompilerWorkspace&, Report&,
                                             const SourceLocation&,
                                             const std::string& scoped_function_name,
                                             const std::string& element_description );

  CompilerWorkspace& workspace;
  Report& report;

  Variables globals;
  Variables locals;
  Variables captures;
  FlowControlScopes break_scopes;
  FlowControlScopes continue_scopes;
  LocalVariableScopes local_scopes;
  LocalVariableScopes capture_scopes;
  // Pushed and popped in visit_user_function. Needs to be a stack
  // because we _do_ have nested functions (via function expressions).
  //
  // Used in visit_identifier. If the variable does not exist under `name`,
  // check `current_scope::name` (if current scope exists).
  std::stack<ScopeName> current_scope_names;
  ScopeName& current_scope_name();

  // Needed to handle super() calls
  std::stack<UserFunction*> user_functions;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
