#ifndef POLSERVER_SEMANTICANALYZER_H
#define POLSERVER_SEMANTICANALYZER_H

#include "compiler/ast/NodeVisitor.h"

#include <map>
#include <memory>

#include "../clib/maputil.h"

#include "FlowControlScopes.h"
#include "Scopes.h"
#include "Variables.h"

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Constants;
class LoopStatement;
class Program;
class ProgramParameterDeclaration;
class ProgramParameterList;
class Report;
class UserFunction;

class SemanticAnalyzer : public NodeVisitor
{
public:
  SemanticAnalyzer( Constants&, Report& );

  ~SemanticAnalyzer() override;

  static void register_const_declarations( CompilerWorkspace& );
  void analyze( CompilerWorkspace& );

  void visit_basic_for_loop( BasicForLoop& node ) override;
  void visit_block( Block& node ) override;
  void visit_break_statement( BreakStatement& node ) override;
  void visit_case_statement( CaseStatement& case_ast ) override;
  void visit_case_dispatch_group( CaseDispatchGroup& dispatch_group ) override;
  void visit_case_dispatch_selectors( CaseDispatchSelectors& selectors ) override;
  void visit_continue_statement( ContinueStatement& node ) override;
  void visit_cstyle_for_loop( CstyleForLoop& loop ) override;
  void visit_do_while_loop( DoWhileLoop& do_while ) override;
  void visit_foreach_loop( ForeachLoop& node ) override;
  void visit_function_call( FunctionCall& fc ) override;
  void visit_function_parameter_list( FunctionParameterList& node ) override;
  void visit_function_parameter_declaration( FunctionParameterDeclaration& node ) override;
  void visit_function_reference( FunctionReference& node ) override;
  void visit_get_member( GetMember& node ) override;
  void visit_identifier( Identifier& node ) override;
  void visit_loop_statement( LoopStatement& loop );
  void visit_module_function_declaration( ModuleFunctionDeclaration& ) override;
  void visit_program( Program& program ) override;
  void visit_program_parameter_list( ProgramParameterList& node ) override;
  void visit_program_parameter_declaration( ProgramParameterDeclaration& node ) override;
  void visit_repeat_until_loop( RepeatUntilLoop& node ) override;
  void visit_user_function( UserFunction& node ) override;
  void visit_var_statement( VarStatement& node ) override;
  void visit_while_loop( WhileLoop& node ) override;

private:
  Constants& constants;
  Report& report;

  Variables globals;
  Variables locals;
  FlowControlScopes break_scopes;
  FlowControlScopes continue_scopes;
  Scopes scopes;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SEMANTICANALYZER_H
