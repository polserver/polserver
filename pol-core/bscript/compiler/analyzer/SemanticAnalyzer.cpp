#include "SemanticAnalyzer.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <ranges>
#include <set>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/analyzer/Constants.h"
#include "bscript/compiler/analyzer/FlowControlScope.h"
#include "bscript/compiler/analyzer/FunctionVariableScope.h"
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
#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ClassInstance.h"
#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/ConstantPredicateLoop.h"
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionExpression.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/GeneratedFunction.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IndexBinding.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"
#include "bscript/compiler/ast/RepeatUntilLoop.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/SequenceBinding.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UninitializedFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/ast/VariableBinding.h"
#include "bscript/compiler/ast/WhileLoop.h"
#include "bscript/compiler/astbuilder/SimpleValueCloner.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/ScopeName.h"
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
      captures( VariableScope::Capture, report ),
      break_scopes( locals, report ),
      continue_scopes( locals, report ),
      local_scopes( locals, report ),
      capture_scopes( captures, report )
{
  current_scope_names.push( ScopeName::Global );
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& workspace, Report& report )
{
  for ( auto& constant : workspace.const_declarations )
  {
    report_function_name_conflict( workspace, report, constant->source_location,
                                   constant->name.string(), "constant" );
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
    // Function expressions are analyzed within visit_function_expression.
    if ( !user_function->expression )
    {
      user_function->accept( *this );
    }
  }

  // Class declarations do not have var statements as children, so we do not get
  // 'duplicate variable' errors.
  for ( auto& class_decl : workspace.class_declarations )
  {
    class_decl->accept( *this );
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

void SemanticAnalyzer::visit_index_binding( IndexBinding& node )
{
  u8 index = 0;

  if ( node.binding_count() > 127 )
  {
    report.error( node, "Too many binding elements. Maximum is 127." );
  }

  for ( const auto& child : node.bindings() )
  {
    if ( auto member_binding = dynamic_cast<VariableBinding*>( &child.get() ) )
    {
      if ( member_binding->rest )
      {
        if ( node.children.back().get() != member_binding )
          report.error( *member_binding, "Index rest binding must be the last in the list." );

        node.rest_index = index;
      }
    }

    ++index;
  }

  visit_children( node );
}

void SemanticAnalyzer::visit_class_declaration( ClassDeclaration& node )
{
  const auto& class_name = node.name;
  std::set<std::string, Clib::ci_cmp_pred> named_baseclasses;
  std::list<ClassDeclaration*> to_visit;
  std::set<ClassDeclaration*> visited;

  report.debug( node, "Class '{}' declared with {} parameters", class_name,
                node.parameters().size() );

  for ( const auto& base_class_link : node.base_class_links )
  {
    const auto& baseclass_name = base_class_link->name;
    auto itr = workspace.all_class_locations.find( baseclass_name );
    if ( itr == workspace.all_class_locations.end() )
    {
      report.error( base_class_link->source_location,
                    "Class '{}' references unknown base class '{}'", class_name, baseclass_name );
    }
    else
    {
      if ( auto cd = base_class_link->class_declaration() )
        to_visit.push_back( cd );
      else
        node.internal_error( "no class linked for base class" );
    }

    if ( Clib::caseInsensitiveEqual( baseclass_name, class_name ) )
    {
      report.error( base_class_link->source_location,
                    "Class '{}' references itself as a base class.", class_name );
    }

    bool previously_referenced =
        named_baseclasses.find( baseclass_name ) != named_baseclasses.end();

    if ( previously_referenced )
    {
      report.error( base_class_link->source_location,
                    "Class '{}' references base class '{}' multiple times.", class_name,
                    baseclass_name );
    }
    else
    {
      named_baseclasses.emplace( baseclass_name );
      report.debug( base_class_link->source_location, "Class '{}' references base class '{}'",
                    class_name, baseclass_name );
    }
  }

  for ( auto to_visit_itr = to_visit.begin(); to_visit_itr != to_visit.end();
        to_visit_itr = to_visit.erase( to_visit_itr ) )
  {
    auto cd = *to_visit_itr;
    if ( visited.find( cd ) != visited.end() )
    {
      continue;
    }

    visited.insert( cd );

    if ( cd == &node )
    {
      report.error( node, "Class '{}' references itself as a base class through inheritance.",
                    class_name );
    }

    for ( const auto& base_class_link : cd->base_class_links )
    {
      if ( auto base_cd = base_class_link->class_declaration() )
      {
        to_visit.push_back( base_cd );
      }
      else
      {
        cd->internal_error( "no class linked for base class" );
      }
    }
  }

  for ( const auto& uninit_function_ref : node.uninit_functions() )
  {
    const auto& uninit_function = uninit_function_ref.get();
    if ( auto exiting_method_itr = node.methods.find( uninit_function.name );
         exiting_method_itr != node.methods.end() )
    {
      report.error( uninit_function.source_location,
                    "In uninitialized function declaration: A method named '{}' is already "
                    "defined in class '{}'.\n"
                    "  See also: {}",
                    uninit_function.name, class_name, exiting_method_itr->second->source_location );
    }
    else if ( uninit_function.type == UserFunctionType::Constructor && node.constructor_link )
    {
      report.error(
          uninit_function.source_location,
          "In uninitialized function declaration: A constructor is already defined in class '{}'.\n"
          "  See also: {}",
          class_name, node.constructor_link->source_location );
    }
  }

  // To visit UninitializedFunctionDeclarations
  visit_children( node );
}

void SemanticAnalyzer::visit_uninitialized_function_declaration(
    UninitializedFunctionDeclaration& node )
{
  if ( Clib::caseInsensitiveEqual( node.name, Compiler::SUPER ) )
  {
    report.error( node, "An uninitialized function cannot be named 'super'." );
  }
  else if ( node.type == UserFunctionType::Static )
  {
    report.error( node.source_location,
                  "In uninitialized function declaration: Static functions cannot be "
                  "marked as uninitialized." );
  }
  else
  {
    // Cannot use visit_children to visit the parameters, since the the
    // SemanticAnalyzer would attempt to make variables for the function inside
    // visit_function_parameter_list.

    bool can_have_rest_parameter = true;
    bool can_have_defaults = true;

    auto params = node.parameters();

    // Rest params must be last, and defaulted params must come after
    // non-defaulted params (excluding rest param).
    for ( auto& param_ref : std::views::reverse( params ) )
    {
      auto& param = param_ref.get();

      if ( param.rest )
      {
        if ( !can_have_rest_parameter )
        {
          report.error( param,
                        "In uninitialized function declaration: Rest parameter must be the last "
                        "parameter in the list." );
        }
        else if ( param.uninit_default )
        {
          report.error( param.source_location,
                        "In uninitialized function declaration: Rest parameter cannot have a "
                        "default value." );
        }
      }
      else if ( param.uninit_default )
      {
        if ( !can_have_defaults )
        {
          report.error( param,
                        "In uninitialized function declaration: Parameters with default values "
                        "must come after all parameters without default values." );
        }
      }
      else
      {
        can_have_defaults = false;
      }

      can_have_rest_parameter = false;
    }
  }
}

void SemanticAnalyzer::analyze_class( ClassDeclaration* class_decl )
{
  if ( analyzed_classes.contains( class_decl ) )
    return;

  // Since only non-static classes (ie. those with constructors) can have uninitialized
  // functions, skip analysis if the class does not have a (possibly inherited) constructor.
  //
  // The check for `user_function()` is just a safety check to prevent null pointer dereference:
  // other checks would report on this not being linked.
  if ( !class_decl->constructor_link || !class_decl->constructor_link->user_function() )
  {
    analyzed_classes.insert( class_decl );
    return;
  }

  std::vector<std::reference_wrapper<UninitializedFunctionDeclaration>> all_uninit_functions;
  std::list<ClassDeclaration*> to_visit{ class_decl };
  std::set<ClassDeclaration*> visited;
  std::map<std::string, UserFunction*, Clib::ci_cmp_pred> all_methods;

  for ( auto* cd : to_visit )
  {
    if ( visited.contains( cd ) )
    {
      continue;
    }

    visited.insert( cd );

    auto cd_uninit_functions = cd->uninit_functions();
    std::ranges::move( cd_uninit_functions, std::back_inserter( all_uninit_functions ) );

    for ( const auto& [method_name, method_link] : cd->methods )
    {
      if ( auto uf = method_link->user_function() )
      {
        // Only add the method if it doesn't already exist: a base class' method
        // should not overwrite a child class' method.
        if ( !all_methods.contains( method_name ) )
        {
          all_methods[method_name] = uf;
        }
      }
      else
      {
        cd->internal_error( fmt::format( "no user function linked for method {}::{}",
                                         class_decl->name, method_name ) );
      }
    }

    for ( const auto& base_class_link : cd->base_class_links )
    {
      if ( auto base_cd = base_class_link->class_declaration() )
      {
        to_visit.push_back( base_cd );
      }
      else
      {
        cd->internal_error( fmt::format( "no class linked for {} baseclass {}", class_decl->name,
                                         base_class_link->name ) );
      }
    }
  }

  auto report_error_if_not_same =
      [&]( UserFunction* defined_func, UninitializedFunctionDeclaration* uninit_func )
  {
    auto defined_params = defined_func->parameters();
    auto uninit_params = uninit_func->parameters();
    auto is_defined_variadic = !defined_params.empty() && defined_params.back().get().rest;
    auto is_uninit_variadic = !uninit_params.empty() && uninit_params.back().get().rest;
    FunctionParameterDeclaration* bad_param = nullptr;
    std::string details;

    if ( is_defined_variadic != is_uninit_variadic ||
         defined_params.size() != uninit_params.size() || defined_func->type != uninit_func->type )
    {
      details =
          fmt::format( "Expecting {} with {}{} parameters, got {} with {}{} parameters.",
                       uninit_func->type, uninit_params.size(), is_uninit_variadic ? "+" : "",
                       defined_func->type, defined_params.size(), is_defined_variadic ? "+" : "" );
    }
    else
    {
      // Size has already been checked for equivalence but keeping both checks for clarity.
      for ( size_t i = 0; i < defined_params.size() && i < uninit_params.size(); ++i )
      {
        auto& defined_param = defined_params[i].get();
        auto& uninit_param = uninit_params[i].get();

        if ( defined_param.byref != uninit_param.byref )
        {
          details = fmt::format(
              "Parameter {} ('{}') is passed {} in uninitialized function but {} in defined "
              "function.",
              i + 1, defined_param.name.string(), uninit_param.byref ? "by reference" : "by value",
              defined_param.byref ? "by reference" : "by value" );
          bad_param = &defined_param;

          break;  // Stop on first error
        }
        else if ( uninit_param.uninit_default && defined_param.default_value() == nullptr )
        {
          details = fmt::format( "Parameter {} ('{}') must have a default value.", i + 1,
                                 defined_param.name.string() );
          bad_param = &defined_param;

          break;  // Stop on first error
        }
      }
    }
    if ( !details.empty() )
    {
      report.error( bad_param ? bad_param->source_location : defined_func->source_location,
                    "Class method '{}' does not correctly implement uninitialized function '{}':\n"
                    "  {}\n"
                    "  See also: {}",
                    defined_func->scoped_name(), uninit_func->scoped_name(), details,
                    uninit_func->source_location );
    }
  };

  for ( const auto& uninit_fun_ref : all_uninit_functions )
  {
    auto& uninit_func = uninit_fun_ref.get();
    report.debug( uninit_func, "Class '{}' inherits uninitialized function '{}::{}'",
                  class_decl->name, uninit_func.scope, uninit_func.name );

    if ( uninit_func.type == UserFunctionType::Constructor )
    {
      report_error_if_not_same( class_decl->constructor_link->user_function(), &uninit_func );
    }
    else if ( auto method_itr = all_methods.find( uninit_func.name );
              method_itr != all_methods.end() )
    {
      report_error_if_not_same( method_itr->second, &uninit_func );
    }
    else if ( auto nonmethod_func = std::ranges::find_if(
                  workspace.user_functions,
                  [&]( const auto& uf )
                  {
                    return Clib::caseInsensitiveEqual( uf->name, uninit_func.name ) &&
                           uf->scope == class_decl->name;
                  } );
              nonmethod_func != workspace.user_functions.end() )
    {
      // This will _always_ error, reporting the defined function as static, and the uninit function
      // as method.
      report_error_if_not_same( nonmethod_func->get(), &uninit_func );
    }
    else
    {
      // A "not implemented" error will happen if the function is static and unreferenced (ie. does
      // not exist in workspace.user_functions but exists in
      // function_resolver.available_user_function_parse_trees).
      report.error( class_decl->source_location,
                    "Class '{}' does not implement uninitialized function '{}'\n"
                    "  See also: {}",
                    class_decl->name, uninit_func.scoped_name(), uninit_func.source_location );
    }
  }

  analyzed_classes.insert( class_decl );
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
    report.error( identifier, "Case selector '{}' is not a constant.", identifier.name() );
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

ScopeName& SemanticAnalyzer::current_scope_name()
{
  return current_scope_names.top();
}

void SemanticAnalyzer::visit_function_call( FunctionCall& fc )
{
  bool is_super_call =
      // The linked function is a SuperFunction
      ( fc.function_link->user_function() &&
        fc.function_link->user_function()->type == UserFunctionType::Super ) ||

      ( !fc.function_link->function() &&  // no linked function
        fc.scoped_name &&                 // there is a name in the call (ie. not an expression)
        Clib::caseInsensitiveEqual( fc.scoped_name->string(),
                                    Compiler::SUPER ) );  // the name is "super"

  // No function linked through FunctionResolver
  if ( !fc.function_link->function() )
  {
    if ( is_super_call && !globals.find( Compiler::SUPER ) && !locals.find( Compiler::SUPER ) )
    {
      report.error( fc, "In call to 'super': No base class defines a constructor." );
      return;  // skip "Unknown identifier" error
    }

    // Method name may be set to variable name, eg: `var foo; foo();` If so,
    // clear it out and insert it at the children start to set as callee.
    if ( fc.scoped_name )
    {
      // If the a function is a class, then it did not define a constructor (since
      // there was no function linked).

      // If a function call is eg. `Animal()` with no scope, check the string as-is.
      std::string class_name = fc.string();

      auto class_itr = workspace.all_class_locations.find( class_name );
      if ( class_itr == workspace.all_class_locations.end() )
      {
        class_name = fc.scoped_name->scope.string();

        class_itr = workspace.all_class_locations.find( class_name );
        if ( class_itr == workspace.all_class_locations.end() )
        {
          class_name.clear();
        }
      }

      if ( !class_name.empty() )
      {
        // There may be a variable named the same as the class, eg:
        //
        //   class Animal() var Animal; endclass
        //
        // If that is the case, there will be a global named
        // `class_name::class_name`. We will not error in this case.
        if ( Clib::caseInsensitiveEqual( fc.scoped_name->name, class_name ) &&
             !globals.find( ScopableName( class_name, class_name ).string() ) )
        {
          bool has_base_classes = false;

          if ( auto class_decl_itr = workspace.class_declaration_indexes.find( class_name );
               class_decl_itr != workspace.class_declaration_indexes.end() &&
               workspace.class_declarations[class_decl_itr->second]->parameters().size() > 0 )
          {
            has_base_classes = true;
          }

          auto msg =
              fmt::format( "In function call: Class '{}' {} define a constructor.", class_name,
                           has_base_classes ? "and its base class(es) do not" : "does not" );

          auto func_itr = workspace.all_function_locations.find(
              ScopableName( class_name, class_name ).string() );

          if ( func_itr != workspace.all_function_locations.end() )
          {
            msg += fmt::format( "\n  See also (missing 'this' parameter?): {}", func_itr->second );
          }

          report.error( fc, msg );
          return;  // skip "Unknown identifier" error
        }
      }

      auto callee = std::make_unique<Identifier>( fc.source_location, *fc.scoped_name );
      fc.children.insert( fc.children.begin(), std::move( callee ) );
      fc.scoped_name.reset();
    }

    // For function calls where the callee is not an identifier, take the
    // arguments as-is. We don't support named args, as we don't know the
    // function to execute until runtime.
    auto any_named = std::find_if( fc.children.begin() + 1, fc.children.end(),
                                   []( const std::unique_ptr<Node>& node )
                                   {
                                     const auto& arg_name =
                                         static_cast<Argument*>( node.get() )->identifier;
                                     return arg_name != nullptr;
                                   } );

    if ( any_named != fc.children.end() )
    {
      report.error( fc, "In function call: Cannot use named arguments here." );

      return;
    }

    visit_children( fc );

    return;
  }

  // here we turn the arguments passed (which can be named or positional)
  // into the final_arguments vector, which is just one parameter per
  // argument, in the correct order.

  typedef std::map<std::string, std::unique_ptr<Expression>> ArgumentList;
  ArgumentList arguments_passed;

  typedef std::list<std::unique_ptr<Expression>> VariadicArguments;
  VariadicArguments variadic_arguments;

  bool any_named = false;
  auto uf = fc.function_link->user_function();

  std::vector<std::unique_ptr<Argument>> arguments = fc.take_arguments();
  auto parameters = fc.parameters();
  bool has_class_inst_parameter = false;

  bool in_super_func = false;
  bool in_constructor_func = false;
  bool in_generated_function = false;

  if ( !user_functions.empty() )
  {
    if ( user_functions.top()->type == UserFunctionType::Super )
    {
      in_super_func = true;
    }
    else if ( user_functions.top()->type == UserFunctionType::Constructor )
    {
      in_constructor_func = true;
      in_generated_function = dynamic_cast<GeneratedFunction*>( user_functions.top() );
    }
  }

  if ( uf )
  {
    // A super() call can only be used in a constructor function.
    if ( is_super_call && !in_constructor_func )
    {
      report.error( fc, "In call to '{}': super() can only be used in constructor functions.",
                    uf->name );
      return;
    }
    // Constructor functions are defined as `Constr( this )` and called
    // statically via `Constr()`. Provide a `this` parameter at this function
    // call site. Only do this when calling constructors outside of a
    // compiler-generated function (ie. super or generated constructor)
    else if ( uf->type == UserFunctionType::Constructor && !in_generated_function &&
              !in_super_func )
    {
      // A super call inherits the `this` argument
      if ( is_super_call )
      {
        // Super will use "this" argument
        arguments.insert( arguments.begin(),
                          std::make_unique<Argument>(
                              fc.source_location,
                              std::make_unique<Identifier>( fc.source_location, "this" ), false ) );

        report.debug( fc, "using ctor Identifier is_super_call={} in_super_func={} uf->name={}",
                      is_super_call, in_super_func, uf->name );
      }
      else
      {
        // Should never happen
        if ( uf->class_link == nullptr || uf->class_link->class_declaration() == nullptr )
        {
          uf->internal_error(
              fmt::format( "no class declaration found for user function '{}'", uf->name ) );
        }

        // Constructor will create a new "this" instance
        arguments.insert( arguments.begin(),
                          std::make_unique<Argument>(
                              fc.source_location,
                              std::make_unique<ClassInstance>(
                                  fc.source_location, uf->class_link->class_declaration() ),
                              false ) );

        report.debug( fc, "using ClassInstance is_super_call={} in_super_func={} uf->name={}",
                      is_super_call, in_super_func, uf->name );

        // Check class for uninit functions defined.
        analyze_class( uf->class_link->class_declaration() );
      }
      // Since a `this` argument is generated for constructor functions, disallow passing an
      // argument named `this`.
      has_class_inst_parameter = true;
    }
    else if ( uf->type == UserFunctionType::Super )
    {
      // Super will use "this" argument
      arguments.insert( arguments.begin(),
                        std::make_unique<Argument>(
                            fc.source_location,
                            std::make_unique<Identifier>( fc.source_location, "this" ), false ) );

      report.debug( fc, "using super Identifier is_super_call={} in_super_func={} uf->name={}",
                    is_super_call, in_super_func, uf->name );
    }
  }

  auto is_callee_variadic = parameters.size() && parameters.back().get().rest;

  const auto method_name = fc.string();

  for ( auto& arg_unique_ptr : arguments )
  {
    auto& arg = *arg_unique_ptr;
    std::string arg_name = arg.identifier ? arg.identifier->string() : "";

    if ( arg.spread )
    {
      if ( !uf )  // a module function
      {
        report.error( arg,
                      "In call to '{}': Spread operator cannot be used in module function call.",
                      method_name );
        return;
      }
      else if ( !uf->is_variadic() )
      {
        report.error( arg,
                      "In call to '{}': Spread operator can only be used in variadic functions.",
                      method_name );
        return;
      }
      else if ( arguments_passed.size() < parameters.size() - 1 )
      {
        report.error( arg,
                      "In call to '{}': Spread operator can only be used for arguments on or after "
                      "the formal rest parameter.",
                      method_name );
        return;
      }
    }

    if ( arg_name.empty() )
    {
      // Allow spread elements to come after named arguments, eg:
      //
      //  `foo( optA := 1, optB := 2, ... c )`
      //
      if ( any_named && !arg.spread )
      {
        report.error( arg, "In call to '{}': Unnamed args cannot follow named args.", method_name );
        return;
      }

      // Too many arguments passed?
      if ( arguments_passed.size() >= parameters.size() )
      {
        // Allowed if variadic
        if ( is_callee_variadic )
        {
          variadic_arguments.push_back( arg.take_expression() );

          // Do not add to `arguments_passed`, so continue.
          continue;
        }
        else
        {
          auto expected_args =
              static_cast<int>( parameters.size() ) - ( has_class_inst_parameter ? 1 : 0 );

          report.error( arg, "In call to '{}': Too many arguments passed.  Expected {}, got {}.",
                        method_name, expected_args, arguments.size() );
          continue;
        }
      }
      else
      {
        arg_name = parameters.at( arguments_passed.size() ).get().name.string();
      }
    }
    else
    {
      any_named = true;

      if ( has_class_inst_parameter && !in_super_func && !is_super_call &&
           Clib::caseInsensitiveEqual( arg_name, "this" ) )
      {
        report.error( arg, "In call to '{}': Cannot pass 'this' to constructor function.",
                      method_name );
        return;
      }
    }

    if ( arguments_passed.find( arg_name ) != arguments_passed.end() )
    {
      report.error( arg, "In call to '{}': Parameter '{}' passed more than once.", method_name,
                    arg_name );
      return;
    }

    // Inside a call to super(), if the arg is un-scoped, find the base class it belongs to. Error
    // if ambiguous.
    if ( is_super_call )
    {
      if ( arg.identifier && arg.identifier->global() && arg.identifier->string() != "this" )
      {
        std::string base_class;
        std::string first_location;
        std::string err_msg;

        auto add_location = [this]( std::string& where, const std::string& class_name )
        {
          fmt::format_to( std::back_inserter( where ), "  See: {}", class_name );

          auto funct_itr = workspace.all_function_locations.find(
              ScopableName( class_name, class_name ).string() );
          if ( funct_itr != workspace.all_function_locations.end() )
          {
            fmt::format_to( std::back_inserter( where ), " {}\n", funct_itr->second );
          }
          else
          {
            where += "\n";
          }
        };

        for ( auto& param_ref : parameters )
        {
          auto& param = param_ref.get();
          const auto& param_name = param.name.name;
          if ( Clib::caseInsensitiveEqual( param_name, arg_name ) )
          {
            if ( !base_class.empty() )
            {
              if ( err_msg.empty() )
              {
                fmt::format_to( std::back_inserter( err_msg ),
                                "In call to '{}': Ambiguous parameter '{}'.\n{}", method_name,
                                param_name, first_location );
              }

              add_location( err_msg, param.name.scope.string() );
            }
            else
            {
              base_class = param.name.scope.string();

              add_location( first_location, base_class );
            }
          }
        }
        if ( !err_msg.empty() )
        {
          report.error( fc, err_msg );
        }
        else
        {
          arg_name = ScopableName( base_class, arg_name ).string();
        }
      }
    }


    arguments_passed[arg_name] = arg.take_expression();
  }

  std::vector<std::unique_ptr<Node>> final_arguments;

  for ( auto& param_ref : parameters )
  {
    FunctionParameterDeclaration& param = param_ref.get();
    auto itr = arguments_passed.find( param.name.string() );
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
              method_name, param.name );
          return;
        }
      }
      else if ( !param.rest )
      {
        report.error( fc,
                      "In call to '{}': Parameter '{}' was not passed, and there is no default.",
                      method_name, param.name );
        return;
      }
    }
    else
    {
      final_arguments.push_back( std::move( ( *itr ).second ) );
      arguments_passed.erase( itr );
    }
  }

  if ( is_callee_variadic )
  {
    // Push the leftover arguments into the call.
    for ( auto& arg : variadic_arguments )
    {
      final_arguments.push_back( std::move( arg ) );
    }
  }
  else
  {
    for ( auto& unused_argument : arguments_passed )
    {
      report.error( *unused_argument.second,
                    "In call to '{}': Parameter '{}' passed by name, but the function has no "
                    "such parameter.",
                    method_name, unused_argument.first );
    }
    if ( !arguments_passed.empty() || arguments.size() > parameters.size() )
      return;
  }

  fc.children = std::move( final_arguments );

  // do this afterwards, so that named parameters will not be looked up as identifiers.
  visit_children( fc );
}

void SemanticAnalyzer::visit_function_parameter_list( FunctionParameterList& node )
{
  // A rest parameter can only be the last parameter in the list. Since we
  // iterate in reverse, start at `true` and set to `false` after first
  // iteration.
  bool can_have_rest_parameter = true;

  for ( auto& child : std::views::reverse( node.children ) )
  {
    child->accept( *this );

    bool has_rest_parameter = static_cast<FunctionParameterDeclaration*>( child.get() )->rest;

    if ( has_rest_parameter && !can_have_rest_parameter )
    {
      report.error( *child, "Rest parameter must be the last parameter in the list." );
    }

    can_have_rest_parameter = false;
  }
}

void SemanticAnalyzer::visit_function_parameter_declaration( FunctionParameterDeclaration& node )
{
  auto node_name = node.name.string();
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
                    node_name );
      // but continue, to avoid unknown identifier errors
    }
  }
  if ( auto existing = locals.find( node_name ) )
  {
    report.error( node, "Parameter '{}' already defined.", node_name );
    return;
  }

  if ( node.rest && node.default_value() )
  {
    report.error( node, "Rest parameter '{}' cannot have a default value.", node_name );
    return;
  }

  WarnOn warn_on = node.unused ? WarnOn::IfUsed : WarnOn::IfNotUsed;

  if ( report_function_name_conflict( node.source_location, node_name, "function parameter" ) )
  {
    warn_on = WarnOn::Never;
  }

  local_scopes.current_local_scope()->create( node_name, warn_on, node.source_location );
}

void SemanticAnalyzer::visit_function_expression( FunctionExpression& node )
{
  if ( auto user_function = node.function_link->user_function() )
  {
    // Create a new capture scope for this function. It must be in a new C++
    // scope for to add the captures to
    // `user_function->capture_variable_scope_info` via the user function
    // visitor.
    {
      FunctionVariableScope new_capture_scope( captures );
      LocalVariableScope capture_scope( capture_scopes,
                                        user_function->capture_variable_scope_info );
      FunctionVariableScope new_function_scope( locals );
      visit_user_function( *user_function );
    }

    // Since the capture_scope was popped (above), any _existing_ capture scope refers to
    // the parent function expression in the tree. Adjust that function to inherit this function's
    // captures.
    if ( auto cap_scope = capture_scopes.current_local_scope() )
    {
      for ( auto& variable : user_function->capture_variable_scope_info.variables )
      {
        // If the capture is not local, we must capture it
        if ( !locals.find( variable->name ) )
        {
          // If already captured, set the variables capture to the existing.
          if ( auto captured = captures.find( variable->name ) )
          {
            variable->capturing = captured;
          }
          // Otherwise, create new.
          else if ( !captures.find( variable->name ) )
          {
            // Create a new capture variable in the parent function, setting
            // this function expression's captured variable to this newly
            // created one.
            variable->capturing = cap_scope->capture( variable->capturing );
          }
        }
      }
    }
  }
}

void SemanticAnalyzer::visit_function_reference( FunctionReference& node )
{
  if ( auto function = node.function_link->user_function() )
  {
    if ( function->type == UserFunctionType::Super )
    {
      report.error( node, "Cannot reference super() function." );
    }
    else if ( function->type == UserFunctionType::Constructor )
    {
      if ( auto class_decl = function->class_link->class_declaration() )
        analyze_class( class_decl );
      else
      {
        // Should never happen, since a constructor function would always have a
        // link to its class.
        function->internal_error( fmt::format(
            "no class declaration found for class constructor '{}'", function->name ) );
      }
    }
  }
  else
  {
    report.error( node, "User function '{}' not found", node.name );
  }
}

void SemanticAnalyzer::visit_identifier( Identifier& node )
{
  // Resolution order:
  //
  // if scoped: locals -> globals
  // otherwise: local function -> local captures -> ancestor (above) functions -> globals
  //
  const auto& name = node.scoped_name.string();

  // If there is a scope, whether it is (":foo") empty or not ("Animal:foo"),
  // we need to check both globals and locals.
  if ( !node.scoped_name.scope.empty() )
  {
    if ( !node.scoped_name.scope.global() )
    {
      if ( auto local = locals.find( name ) )
      {
        local->mark_used();
        node.variable = local;
      }
    }
    // Did not find it in locals, check globals
    if ( !node.variable )
    {
      if ( auto scoped_global = globals.find( name ) )
      {
        node.variable = scoped_global;
      }
    }
  }
  else
  {
    if ( auto local = locals.find( name ) )
    {
      local->mark_used();
      node.variable = local;
    }
    else if ( auto captured = captures.find( name ) )
    {
      // Should already be marked used as it's not newly created (done below).
      // There is no `captures.find_in_ancestors()` check because if an upper
      // capture was found, we still need to capture it for our own function (done
      // below).
      node.variable = captured;
    }
    else if ( auto ancestor = locals.find_in_ancestors( name ) )
    {
      // Capture the variable. In a deeply nested capture, this will reference the
      // local in the ancestor function. The function expression visitor will swap
      // the 'capturing' to a local-safe variable.
      node.variable = capture_scopes.current_local_scope()->capture( ancestor );
      node.variable->mark_used();
    }
    else if ( auto global = globals.find( name ) )
    {
      node.variable = global;
    }
    else if ( !current_scope_name().global() )
    {
      const auto scoped_name = ScopableName( current_scope_name(), node.name() ).string();

      // We do not support nested classes, so if there is a `current_scope`, it would only _ever_
      // exist in globals.
      if ( auto scoped_global = globals.find( scoped_name ) )
      {
        node.variable = scoped_global;
      }
    }
  }

  if ( !node.variable )
  {
    report.error( node, "Unknown identifier '{}'.", name );
    return;
  }
}

void SemanticAnalyzer::visit_variable_binding( VariableBinding& node )
{
  if ( auto variable = create_variable( node.source_location, node.scoped_name.scope.string(),
                                        node.scoped_name.name ) )
  {
    node.variable = std::move( variable );
    visit_children( node );
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

void SemanticAnalyzer::visit_return_statement( ReturnStatement& node )
{
  if ( !user_functions.empty() )
  {
    auto uf = user_functions.top();

    if ( uf->type == UserFunctionType::Constructor && !node.children.empty() )
    {
      report.error( node, "Cannot return a value from a constructor function." );
    }
  }

  visit_children( node );
}

void SemanticAnalyzer::visit_sequence_binding( SequenceBinding& node )
{
  u8 index = 0;
  VariableBinding* previous_rest_binding = nullptr;

  if ( node.binding_count() > 127 )
  {
    report.error( node, "Too many binding elements. Maximum is 127." );
  }

  for ( const auto& child : node.children )
  {
    if ( auto index_binding = dynamic_cast<VariableBinding*>( child.get() ) )
    {
      if ( index_binding->rest )
      {
        if ( previous_rest_binding != nullptr )
        {
          report.error( *index_binding,
                        "Only one rest binding is allowed.\n"
                        "  See also: {}",
                        previous_rest_binding->source_location );
        }

        previous_rest_binding = index_binding;
        node.rest_index = index;
      }
    }

    ++index;
  }

  visit_children( node );
}

void SemanticAnalyzer::visit_user_function( UserFunction& node )
{
  // Track current scope for use in visit_identifier
  current_scope_names.push( ScopeName( node.scope ) );
  user_functions.emplace( &node );
  if ( node.exported )
  {
    if ( !node.scope.empty() )
    {
      report.error( node, "Exported function '{}' cannot be scoped.", node.scoped_name() );
    }
    else
    {
      unsigned max_name_length = sizeof( Pol::Bscript::BSCRIPT_EXPORTED_FUNCTION::funcname ) - 1;
      if ( node.name.length() > max_name_length )
      {
        report.error( node,
                      "Exported function name '{}' is too long at {} characters.  Max length: {}",
                      node.name, node.name.length(), max_name_length );
      }
    }
  }

  LocalVariableScope scope( local_scopes, node.local_variable_scope_info );
  visit_children( node );
  user_functions.pop();
  current_scope_names.pop();
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  if ( auto variable = create_variable( node.source_location, node.scope, node.name ) )
  {
    node.variable = std::move( variable );
    visit_children( node );
  }
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
                          node.identifier().name() );
        }
      }
    }
  }
}

void SemanticAnalyzer::visit_while_loop( WhileLoop& node )
{
  visit_loop_statement( node );
}

void SemanticAnalyzer::visit_constant_loop( ConstantPredicateLoop& node )
{
  visit_loop_statement( node );
}

std::shared_ptr<Variable> SemanticAnalyzer::create_variable( const SourceLocation& source_location,
                                                             const std::string& scope,
                                                             const std::string& name )
{
  auto maybe_scoped_name = ScopableName( scope, name ).string();

  // Since this is not scoped check, we cannot have `Animal::FOO` and a constant `FOO`.
  if ( auto constant = workspace.constants.find( name ) )
  {
    report.error( source_location,
                  "Cannot define a variable with the same name as constant '{}'.\n"
                  "  See also: {}",
                  name, constant->source_location );
    return {};
  }

  report_function_name_conflict( source_location, maybe_scoped_name, "variable" );

  if ( auto local_scope = local_scopes.current_local_scope() )
  {
    return local_scope->create( maybe_scoped_name, WarnOn::Never, source_location );
  }
  else
  {
    if ( auto existing = globals.find( maybe_scoped_name ) )
    {
      report.error( source_location,
                    "Global variable '{}' already defined.\n"
                    "  See also: {}",
                    maybe_scoped_name, existing->source_location );
      return {};
    }

    return globals.create( maybe_scoped_name, 0, WarnOn::Never, source_location );
  }
}

bool SemanticAnalyzer::report_function_name_conflict( const SourceLocation& referencing_loc,
                                                      const std::string& function_name,
                                                      const std::string& element_description )
{
  return report_function_name_conflict(
      workspace, report, referencing_loc,
      ScopableName( current_scope_name(), function_name ).string(), element_description );
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
