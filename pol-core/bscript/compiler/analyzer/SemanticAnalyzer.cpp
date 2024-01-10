#include "SemanticAnalyzer.h"

#include <boost/range/adaptor/reversed.hpp>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/analyzer/Constants.h"
#include "bscript/compiler/analyzer/FlowControlScope.h"
#include "bscript/compiler/analyzer/LocalVariableScope.h"
#include "bscript/compiler/analyzer/LocalVariableScopes.h"
#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/BasicForLoop.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/CaseDispatchDefaultSelector.h"
#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/CaseDispatchGroups.h"
#include "bscript/compiler/ast/CaseDispatchSelectors.h"
#include "bscript/compiler/ast/CaseStatement.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"
#include "bscript/compiler/ast/RepeatUntilLoop.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/ast/WhileLoop.h"
#include "bscript/compiler/astbuilder/SimpleValueCloner.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/Variable.h"
#include "bscript/compiler/optimizer/ConstantValidator.h"
#include "clib/strutil.h"
#include "filefmt.h"

namespace Pol::Bscript::Compiler
{
SemanticAnalyzer::SemanticAnalyzer( CompilerWorkspace& workspace, Report& report )
    : workspace( workspace ),
      report( report ),
      globals( VariableScope::Global, report ),
      locals( VariableScope::Local, report ),
      break_scopes( locals, report ),
      continue_scopes( locals, report ),
      local_scopes( locals, report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& workspace, Report& report )
{
  for ( auto& constant : workspace.const_declarations )
  {
    report_function_name_conflict( workspace, report, constant->source_location,
                                   constant->identifier, "constant" );
    workspace.constants.create( *constant );
  }
}

void SemanticAnalyzer::analyze()
{
  workspace.top_level_statements->accept( *this );
  if ( auto& program = workspace.program )
  {
    program->accept( *this );
  }

  for ( auto& user_function : workspace.user_functions )
  {
    user_function->accept( *this );
  }

  workspace.global_variable_names = globals.get_names();
}

void SemanticAnalyzer::visit_basic_for_loop( BasicForLoop& node )
{
  if ( locals.find( node.identifier ) )
  {
    report.error( node, "FOR iterator '{}' hides a local variable.", node.identifier );
    return;
  }
  if ( report_function_name_conflict( node.source_location, node.identifier, "for loop iterator" ) )
  {
    return;
  }

  node.first().accept( *this );
  node.last().accept( *this );

  LocalVariableScope scope( local_scopes, node.local_variable_scope_info );
  scope.create( node.identifier, WarnOn::Never, node.source_location );
  scope.create( "_" + node.identifier + "_end", WarnOn::Never, node.source_location );

  FlowControlScope break_scope( break_scopes, node.source_location, node.get_label(),
                                node.break_label );
  FlowControlScope continue_scope( continue_scopes, node.source_location, node.get_label(),
                                   node.continue_label );

  node.block().accept( *this );
}

void SemanticAnalyzer::visit_block( Block& block )
{
  LocalVariableScope scope( local_scopes, block.local_variable_scope_info );

  visit_children( block );
}

class CaseDispatchDuplicateSelectorAnalyzer : public NodeVisitor
{
public:
  explicit CaseDispatchDuplicateSelectorAnalyzer( Report& report ) : report( report ) {}

  void visit_block( Block& ) override
  {
    // just don't recurse into children
  }

  void visit_integer_value( IntegerValue& node ) override
  {
    auto seen = already_seen_integers.find( node.value );
    if ( seen != already_seen_integers.end() )
    {
      report.error( node,
                    "case statement already has a selector for integer value {}.\n"
                    "  See also: {}",
                    node.value, ( *seen ).second->source_location );
    }
    else
    {
      already_seen_integers[node.value] = &node;
    }
  }

  void visit_string_value( StringValue& node ) override
  {
    auto seen = already_seen_strings.find( node.value );
    if ( seen != already_seen_strings.end() )
    {
      report.error( node,
                    "case statement already has a selector for string value {}.\n"
                    "  See also: {}",
                    Clib::getencodedquotedstring( node.value ), ( *seen ).second->source_location );
    }
    else
    {
      already_seen_strings[node.value] = &node;
    }
  }

  void visit_case_dispatch_default_selector( CaseDispatchDefaultSelector& node ) override
  {
    if ( already_seen_default )
    {
      report.error( node,
                    "case statement already has a default clause.\n"
                    "  See also: {}",
                    already_seen_default->source_location );
    }
    else
    {
      already_seen_default = &node;
    }
  }

private:
  Report& report;

  CaseDispatchDefaultSelector* already_seen_default = nullptr;
  std::map<int, IntegerValue*> already_seen_integers;
  std::map<std::string, StringValue*> already_seen_strings;
};


void SemanticAnalyzer::visit_case_statement( CaseStatement& case_ast )
{
  CaseDispatchDuplicateSelectorAnalyzer duplicate_detector( report );
  case_ast.dispatch_groups().accept( duplicate_detector );

  FlowControlScope break_scope( break_scopes, case_ast.source_location, case_ast.get_label(),
                                case_ast.break_label );

  visit_children( case_ast );
}

void SemanticAnalyzer::visit_case_dispatch_group( CaseDispatchGroup& dispatch_group )
{
  FlowControlScope break_scope( break_scopes, dispatch_group.source_location, "",
                                dispatch_group.break_label );

  visit_children( dispatch_group );
}

class CaseDispatchSelectorAnalyzer : public NodeVisitor
{
public:
  explicit CaseDispatchSelectorAnalyzer( Report& report ) : report( report ) {}

  void visit_identifier( Identifier& identifier ) override
  {
    report.error( identifier, "Case selector '{}' is not a constant.", identifier.name );
  }

  void visit_string_value( StringValue& sv ) override
  {
    if ( sv.value.size() >= 254 )
    {
      report.error( sv, "String expressions in CASE statements must be <= 253 characters." );
    }
  }

private:
  Report& report;
};

void SemanticAnalyzer::visit_case_dispatch_selectors( CaseDispatchSelectors& selectors )
{
  visit_children( selectors );

  CaseDispatchSelectorAnalyzer selector_analyzer( report );
  selectors.accept( selector_analyzer );
}

void SemanticAnalyzer::visit_cstyle_for_loop( CstyleForLoop& loop )
{
  visit_loop_statement( loop );
}

void SemanticAnalyzer::visit_do_while_loop( DoWhileLoop& do_while )
{
  visit_loop_statement( do_while );
}

void SemanticAnalyzer::visit_foreach_loop( ForeachLoop& node )
{
  if ( report_function_name_conflict( node.source_location, node.iterator_name,
                                      "foreach iterator" ) )
  {
    return;
  }

  node.expression().accept( *this );

  LocalVariableScope scope( local_scopes, node.local_variable_scope_info );
  scope.create( node.iterator_name, WarnOn::Never, node.source_location );
  scope.create( "_" + node.iterator_name + "_expr", WarnOn::Never, node.source_location );
  scope.create( "_" + node.iterator_name + "_iter", WarnOn::Never, node.source_location );

  FlowControlScope break_scope( break_scopes, node.source_location, node.get_label(),
                                node.break_label );
  FlowControlScope continue_scope( continue_scopes, node.source_location, node.get_label(),
                                   node.continue_label );

  node.block().accept( *this );
}

void SemanticAnalyzer::visit_function_call( FunctionCall& fc )
{
  // here we turn the arguments passed (which can be named or positional)
  // into the final_arguments vector, which is just one parameter per
  // argument, in the correct order.

  typedef std::map<std::string, std::unique_ptr<Expression>> ArgumentList;
  ArgumentList arguments_passed;

  bool any_named = false;

  std::vector<std::unique_ptr<Argument>> arguments = fc.take_arguments();
  auto parameters = fc.parameters();

  for ( auto& arg_unique_ptr : arguments )
  {
    auto& arg = *arg_unique_ptr;
    std::string arg_name = arg.identifier;
    if ( arg_name.empty() )
    {
      if ( any_named )
      {
        report.error( arg, "In call to '{}': Unnamed args cannot follow named args.",
                      fc.method_name );
        return;
      }

      if ( arguments_passed.size() >= parameters.size() )
      {
        report.error( arg, "In call to '{}': Too many arguments passed.  Expected {}, got {}.",
                      fc.method_name, parameters.size(), arguments.size() );
        continue;
      }

      arg_name = parameters.at( arguments_passed.size() ).get().name;
    }
    else
    {
      any_named = true;
    }
    if ( arguments_passed.find( arg_name ) != arguments_passed.end() )
    {
      report.error( arg, "In call to '{}': Parameter '{}' passed more than once.", fc.method_name,
                    arg_name );
      return;
    }

    arguments_passed[arg_name] = arg.take_expression();
  }

  std::vector<std::unique_ptr<Node>> final_arguments;

  for ( auto& param_ref : parameters )
  {
    FunctionParameterDeclaration& param = param_ref.get();
    auto itr = arguments_passed.find( param.name );
    if ( itr == arguments_passed.end() )
    {
      if ( auto default_value = param.default_value() )
      {
        SimpleValueCloner cloner( report, default_value->source_location );
        auto final_argument = cloner.clone( *default_value );

        if ( final_argument )
        {
          final_arguments.push_back( std::move( final_argument ) );
        }
        else
        {
          report.error(
              param, "In call to '{}': Unable to create argument from default for parameter '{}'.",
              fc.method_name, param.name );
          return;
        }
      }
      else
      {
        report.error( fc,
                      "In call to '{}': Parameter '{}' was not passed, and there is no default.",
                      fc.method_name, param.name );
        return;
      }
    }
    else
    {
      final_arguments.push_back( std::move( ( *itr ).second ) );
      arguments_passed.erase( itr );
    }
  }

  for ( auto& unused_argument : arguments_passed )
  {
    report.error(
        *unused_argument.second,
        "In call to '{}': Parameter '{}' passed by name, but the function has no such parameter.",
        fc.method_name, unused_argument.first );
  }
  if ( !arguments_passed.empty() || arguments.size() > parameters.size() )
    return;

  fc.children = std::move( final_arguments );

  // do this afterwards, so that named parameters will not be looked up as identifiers.
  visit_children( fc );
}

void SemanticAnalyzer::visit_function_parameter_list( FunctionParameterList& node )
{
  for ( auto& child : boost::adaptors::reverse( node.children ) )
  {
    child->accept( *this );
  }
}

void SemanticAnalyzer::visit_function_parameter_declaration( FunctionParameterDeclaration& node )
{
  if ( auto default_value = node.default_value() )
  {
    ConstantValidator validator;
    // By accident, 0-parameter system function calls are allowed as constant values.
    // They are not allowed as default parameters, though.
    if ( !validator.validate( *default_value ) || dynamic_cast<FunctionCall*>( default_value ) )
    {
      report.error( node,
                    "Parameter '{}' has a disallowed default.  Only simple operands are allowed as "
                    "default arguments.",
                    node.name );
      // but continue, to avoid unknown identifier errors
    }
  }
  if ( auto existing = locals.find( node.name ) )
  {
    report.error( node, "Parameter '{}' already defined.", node.name );
    return;
  }
  WarnOn warn_on = node.unused ? WarnOn::IfUsed : WarnOn::IfNotUsed;

  if ( report_function_name_conflict( node.source_location, node.name, "function parameter" ) )
  {
    warn_on = WarnOn::Never;
  }

  local_scopes.current_local_scope()->create( node.name, warn_on, node.source_location );
}

void SemanticAnalyzer::visit_function_reference( FunctionReference& node )
{
  if ( !node.function_link->function() )
  {
    report.error( node, "User function '{}' not found", node.name );
  }
}

void SemanticAnalyzer::visit_identifier( Identifier& node )
{
  if ( auto local = locals.find( node.name ) )
  {
    local->mark_used();
    node.variable = local;
  }
  else if ( auto global = globals.find( node.name ) )
  {
    node.variable = global;
  }
  else
  {
    report.error( node, "Unknown identifier '{}'.", node.name );
    return;
  }
}

void SemanticAnalyzer::visit_jump_statement( JumpStatement& node )
{
  auto& scopes = node.jump_type == JumpStatement::Break ? break_scopes : continue_scopes;

  if ( auto scope = scopes.find( node.label ) )
  {
    node.flow_control_label = scope->flow_control_label;
    node.local_variables_to_remove = locals.count() - scope->local_variables_size;
  }
  else
  {
    auto type_str = node.jump_type == JumpStatement::Break ? "break" : "continue";

    if ( !node.label.empty() && break_scopes.any() )
      report.error( node, "Label '{}' not found for {}", node.label, type_str );
    else
      report.error( node, "Cannot {} here.", type_str );
  }
}


void SemanticAnalyzer::visit_loop_statement( LoopStatement& loop )
{
  FlowControlScope continue_scope( continue_scopes, loop.source_location, loop.get_label(),
                                   loop.continue_label );
  FlowControlScope break_scope( break_scopes, loop.source_location, loop.get_label(),
                                loop.break_label );

  visit_children( loop );
}

void SemanticAnalyzer::visit_program( Program& program )
{
  LocalVariableScope scope( local_scopes, program.local_variable_scope_info );

  visit_children( program );
}

void SemanticAnalyzer::visit_program_parameter_declaration( ProgramParameterDeclaration& node )
{
  if ( auto existing = locals.find( node.name ) )
  {
    report.error( node, "Parameter '{}' already defined.", node.name );
    return;
  }
  WarnOn warn_on = node.unused ? WarnOn::IfUsed : WarnOn::IfNotUsed;

  if ( report_function_name_conflict( node.source_location, node.name, "program parameter" ) )
  {
    warn_on = WarnOn::Never;
  }

  local_scopes.current_local_scope()->create( node.name, warn_on, node.source_location );
}

void SemanticAnalyzer::visit_repeat_until_loop( RepeatUntilLoop& node )
{
  visit_loop_statement( node );
}

void SemanticAnalyzer::visit_user_function( UserFunction& node )
{
  if ( node.exported )
  {
    unsigned max_name_length = sizeof( Pol::Bscript::BSCRIPT_EXPORTED_FUNCTION::funcname ) - 1;
    if ( node.name.length() > max_name_length )
    {
      report.error( node,
                    "Exported function name '{}' is too long at {} characters.  Max length: {}",
                    node.name, node.name.length(), max_name_length );
    }
  }
  LocalVariableScope scope( local_scopes, node.local_variable_scope_info );

  visit_children( node );
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  if ( auto constant = workspace.constants.find( node.name ) )
  {
    report.error( node,
                  "Cannot define a variable with the same name as constant '{}'.\n"
                  "  See also: {}",
                  node.name, constant->source_location );
    return;
  }

  report_function_name_conflict( node.source_location, node.name, "variable" );

  if ( auto local_scope = local_scopes.current_local_scope() )
  {
    node.variable = local_scope->create( node.name, WarnOn::Never, node.source_location );
  }
  else
  {
    if ( auto existing = globals.find( node.name ) )
    {
      report.error( node,
                    "Global variable '{}' already defined.\n"
                    "  See also: {}",
                    node.name, existing->source_location );
      return;
    }

    node.variable = globals.create( node.name, 0, WarnOn::Never, node.source_location );
  }
  visit_children( node );
}

void SemanticAnalyzer::visit_variable_assignment_statement( VariableAssignmentStatement& node )
{
  visit_children( node );

  if ( auto bop = dynamic_cast<BinaryOperator*>( &node.rhs() ) )
  {
    if ( bop->token_id == TOK_ASSIGN )
    {
      if ( auto second_ident = dynamic_cast<Identifier*>( &bop->lhs() ) )
      {
        if ( node.identifier().variable == second_ident->variable )
        {
          // we have something like
          //      a := a := expr;
          report.warning( node, "Double-assignment to the same variable '{}'.",
                          node.identifier().name );
        }
      }
    }
  }
}

void SemanticAnalyzer::visit_while_loop( WhileLoop& node )
{
  visit_loop_statement( node );
}

bool SemanticAnalyzer::report_function_name_conflict( const SourceLocation& referencing_loc,
                                                      const std::string& function_name,
                                                      const std::string& element_description )
{
  return report_function_name_conflict( workspace, report, referencing_loc, function_name,
                                        element_description );
}

bool SemanticAnalyzer::report_function_name_conflict( const CompilerWorkspace& workspace,
                                                      Report& report,
                                                      const SourceLocation& referencing_loc,
                                                      const std::string& function_name,
                                                      const std::string& element_description )
{
  auto func_itr = workspace.all_function_locations.find( function_name );
  if ( func_itr != workspace.all_function_locations.end() )
  {
    const SourceLocation& function_loc = ( *func_itr ).second;
    report.error( referencing_loc,
                  "Cannot define a {} with the same name as function '{}'.\n"
                  "  Defined here: {}",
                  element_description, function_name, function_loc );
    return true;
  }
  return false;
}

}  // namespace Pol::Bscript::Compiler
