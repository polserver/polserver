#include "SemanticAnalyzer.h"

#include <boost/range/adaptor/reversed.hpp>
#include <list>
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
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionExpression.h"
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
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/ast/WhileLoop.h"
#include "bscript/compiler/astbuilder/SimpleValueCloner.h"
#include "bscript/compiler/model/ClassLink.h"
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

  // Take any generated super() functions during semantic analysis.
  for ( auto& class_decl : workspace.class_declarations )
  {
    auto super = class_decl->take_super();
    // Create a new function variable scope. `super` is almost like a captured
    // function.
    if ( super )
    {
      FunctionVariableScope new_function_scope( locals );
      visit_user_function( *super );
      workspace.user_functions.push_back( std::move( super ) );
    }
  }
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

void SemanticAnalyzer::visit_class_parameter_declaration( ClassParameterDeclaration& node )
{
  auto constructor = node.constructor_link->function();
  if ( !constructor )
  {
    auto msg = fmt::format( "Base class '{}' does not define a constructor.", node.name );

    // Check if the named function exists. If it does, then the reason it wasn't
    // linked is because it is not a constructor. This happens because of a
    // missing `this` parameter.
    auto func_itr =
        workspace.all_function_locations.find( ScopableName( node.name, node.name ).string() );

    if ( func_itr != workspace.all_function_locations.end() )
    {
      msg += fmt::format( "\n  See also (missing 'this' parameter?): {}", func_itr->second );
    }

    report.error( node, msg );
  }
  else
  {
    report.debug( node, "Class parameter '{}' references constructor '{}' at {}", node.name,
                  constructor->scoped_name(), constructor->source_location );
  }
}

void SemanticAnalyzer::visit_class_declaration( ClassDeclaration& node )
{
  const auto& class_name = node.name;

  // Will need the order for something, i'm sure...
  std::vector<std::string> ordered_baseclasses;
  std::set<std::string, Clib::ci_cmp_pred> named_baseclasses;
  report.debug( node, "Class '{}' declared with {} parameters", class_name,
                node.parameters().size() );

  for ( auto& class_parameter : node.parameters() )
  {
    const auto& baseclass_name = class_parameter.get().name;
    auto itr = workspace.all_class_locations.find( baseclass_name );
    if ( itr == workspace.all_class_locations.end() )
    {
      report.error( class_parameter.get(), "Class '{}' references unknown base class '{}'",
                    class_name, baseclass_name );
    }

    if ( baseclass_name == class_name )
    {
      report.error( class_parameter.get(), "Class '{}' references itself as a base class.",
                    class_name );
    }

    bool previously_referenced =
        named_baseclasses.find( baseclass_name ) != named_baseclasses.end();

    if ( previously_referenced )
    {
      report.error( class_parameter.get(), "Class '{}' references base class '{}' multiple times.",
                    class_name, baseclass_name );
    }
    else
    {
      ordered_baseclasses.push_back( baseclass_name );
      named_baseclasses.emplace( baseclass_name );
      report.debug( class_parameter.get(), "Class '{}' references base class '{}'", class_name,
                    baseclass_name );
      visit_class_parameter_declaration( class_parameter.get() );
    }
  }
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

void SemanticAnalyzer::prepare_super_call( FunctionCall& fc )
{
  if ( user_functions.empty() )
  {
    report.error( fc, "In function call: super() can only be used in constructor functions." );
    return;
  }

  auto uf = user_functions.top();

  if ( uf->type != UserFunctionType::Constructor )
  {
    report.error( fc, "In function call: super() can only be used in constructor functions." );
    return;
  }

  auto cd = uf->class_link->class_declaration();

  if ( !cd )
  {
    uf->internal_error( "no class declaration" );
    return;
  }

  // Will return nullptr if super cannot be made, eg. if one of the base classes
  // does not define a constructor.
  auto super = cd->make_super( fc.source_location );

  // Linking to nullptr will cause the `visit_function_call()` for super() to be
  // unlinked, resulting in a compile error.
  fc.function_link->link_to( super );
}

ScopeName& SemanticAnalyzer::current_scope_name()
{
  return current_scope_names.top();
}

void SemanticAnalyzer::visit_function_call( FunctionCall& fc )
{
  bool is_super_call =
      !fc.function_link->function() &&  // no linked function
      fc.scoped_name &&                 // there is a name in the call (ie. not an expression)
      Clib::caseInsensitiveEqual( fc.scoped_name->string(), "super" );  // the name is "super"

  if ( is_super_call )
  {
    // Will add a function link on success
    prepare_super_call( fc );
  }

  // No function linked through FunctionResolver
  if ( !fc.function_link->function() )
  {
    // Method name may be set to variable name, eg: `var foo; foo();` If so,
    // clear it out and insert it at the children start to set as callee.
    if ( fc.scoped_name )
    {
      // If we tried to make a super and it failed, then we received an error
      // because of some other reason (eg. base-class does not implement
      // constructor.)
      if ( is_super_call )
      {
        return;
      }

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
        if ( !globals.find( ScopableName( class_name, class_name ).string() ) )
        {
          auto msg = fmt::format( "In function call: Class '{}' does not define a constructor.",
                                  class_name );

          auto func_itr = workspace.all_function_locations.find(
              ScopableName( class_name, class_name ).string() );

          if ( func_itr != workspace.all_function_locations.end() )
          {
            msg += fmt::format( "\n  See also (missing 'this' parameter?): {}", func_itr->second );
          }

          report.error( fc, msg );
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

  bool in_super_func =
      !user_functions.empty() && user_functions.top()->type == UserFunctionType::Super;

  if ( uf )
  {
    // Constructor functions are defined as `Constr( this )` and called statically via `Constr()`.
    // Provide a `this` parameter at this function call site.
    if ( uf->type == UserFunctionType::Constructor && !in_super_func )
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
        // Constructor will create a new "this" instance
        arguments.insert( arguments.begin(),
                          std::make_unique<Argument>(
                              fc.source_location,
                              std::make_unique<ClassInstance>( fc.source_location ), false ) );

        report.debug( fc, "using ClassInstance is_super_call={} in_super_func={} uf->name={}",
                      is_super_call, in_super_func, uf->name );
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

  for ( auto& child : boost::adaptors::reverse( node.children ) )
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
  if ( !node.function_link->function() )
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

  // Do not error if accessing a class name that does not define a constructor,
  // as that is handled by visit_function_call.
  if ( !node.variable &&
       workspace.all_class_locations.find( name ) == workspace.all_class_locations.end() )
  {
    report.error( node, "Unknown identifier '{}'.", name );
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

void SemanticAnalyzer::visit_user_function( UserFunction& node )
{
  // Track current scope for use in visit_identifier
  current_scope_names.push( ScopeName( node.scope ) );
  user_functions.emplace( &node );
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
  // We do not allow nested scope names
  // TODO fix this because of funcexprs
  user_functions.pop();
  current_scope_names.pop();
}

void SemanticAnalyzer::visit_var_statement( VarStatement& node )
{
  // A scoped variable's `name` will be `scope::name` if a scope exists,
  // otherwise just `name`.
  auto maybe_scoped_name = ScopableName( node.scope, node.name ).string();

  // node.scope.empty() ? node.name : fmt::format( "{}::{}", node.scope, node.name );

  // Since this is not scoped check, we cannot have `Animal::FOO` and a constant `FOO`.
  if ( auto constant = workspace.constants.find( node.name ) )
  {
    report.error( node,
                  "Cannot define a variable with the same name as constant '{}'.\n"
                  "  See also: {}",
                  node.name, constant->source_location );
    return;
  }

  report_function_name_conflict( node.source_location, maybe_scoped_name, "variable" );

  if ( auto local_scope = local_scopes.current_local_scope() )
  {
    node.variable = local_scope->create( maybe_scoped_name, WarnOn::Never, node.source_location );
  }
  else
  {
    if ( auto existing = globals.find( maybe_scoped_name ) )
    {
      report.error( node,
                    "Global variable '{}' already defined.\n"
                    "  See also: {}",
                    maybe_scoped_name, existing->source_location );
      return;
    }

    node.variable = globals.create( maybe_scoped_name, 0, WarnOn::Never, node.source_location );
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
