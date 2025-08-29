#include "GeneratedFunctionBuilder.h"

#include <list>
#include <ranges>
#include <set>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/GeneratedFunction.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/SpreadElement.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/SimpleValueCloner.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
GeneratedFunctionBuilder::GeneratedFunctionBuilder( const SourceFileIdentifier& loc,
                                                    BuilderWorkspace& workspace )
    : CompoundStatementBuilder( loc, workspace )
{
}

void GeneratedFunctionBuilder::super_function( std::unique_ptr<GeneratedFunction>& super )
{
  std::vector<UserFunction*> base_class_ctors;
  std::set<ClassDeclaration*> visited;
  std::list<std::shared_ptr<ClassLink>> to_visit;
  auto class_declaration = super->class_declaration();

  to_visit.insert( to_visit.end(), class_declaration->base_class_links.begin(),
                   class_declaration->base_class_links.end() );

  for ( auto to_link_itr = to_visit.begin(); to_link_itr != to_visit.end();
        to_link_itr = to_visit.erase( to_link_itr ) )
  {
    if ( auto base_cd = ( *to_link_itr )->class_declaration() )
    {
      if ( visited.find( base_cd ) != visited.end() )
      {
        continue;
      }
      visited.insert( base_cd );

      if ( auto constructor_link = base_cd->constructor_link )
      {
        if ( auto base_class_ctor = constructor_link->user_function() )
        {
          base_class_ctors.push_back( base_class_ctor );
        }
      }
    }
  }

  build( super, base_class_ctors );
}

void GeneratedFunctionBuilder::constructor_function(
    std::unique_ptr<GeneratedFunction>& constructor )
{
  std::set<ClassDeclaration*> visited;
  std::list<std::shared_ptr<ClassLink>> to_visit;
  auto class_declaration = constructor->class_declaration();
  UserFunction* base_class_ctor = nullptr;

  to_visit.insert( to_visit.end(), class_declaration->base_class_links.begin(),
                   class_declaration->base_class_links.end() );

  for ( auto to_link_itr = to_visit.begin(); to_link_itr != to_visit.end();
        to_link_itr = to_visit.erase( to_link_itr ) )
  {
    if ( auto base_cd = ( *to_link_itr )->class_declaration() )
    {
      if ( visited.find( base_cd ) != visited.end() )
      {
        continue;
      }
      visited.insert( base_cd );

      if ( auto constructor_link = base_cd->constructor_link )
      {
        if ( auto ctor = constructor_link->user_function() )
        {
          base_class_ctor = ctor;
          break;
        }
      }
    }
  }

  if ( base_class_ctor != nullptr )
  {
    build( constructor, { base_class_ctor } );
  }

  // Set the generated function as the class' constructor function.
  class_declaration->constructor_link =
      std::make_unique<FunctionLink>( constructor->source_location, "", true );
  class_declaration->constructor_link->link_to( constructor.get() );
}

void GeneratedFunctionBuilder::build( std::unique_ptr<GeneratedFunction>& function,
                                      std::vector<UserFunction*> base_class_ctors )
{
  const auto& loc = function->source_location;
  // If there are no base ctors, skip updating the function's parameters and
  // body, therefore a super function that is "invalid" will have no parameters
  // or body.
  if ( !base_class_ctors.empty() )
  {
    auto& function_parameters = function->child<FunctionParameterList>( 0 ).children;

    function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
        loc, ScopableName( ScopeName::None, "this" ), true /* byref */, false /* unused */,
        false /* rest */ ) );

    // Our super() alias'ed parameter can be a rest parameter only if the not-last
    // constructors are not variadic.
    bool can_use_rest = true;

    for ( const auto* base_class_ctor :
          base_class_ctors | std::views::take( base_class_ctors.size() - 1 ) )
    {
      if ( base_class_ctor->is_variadic() )
      {
        can_use_rest = false;
        break;
      }
    }

    std::set<ScopableName> visited_arg_names;

    for ( auto base_class_ctor : base_class_ctors )
    {
      add_base_constructor( function, base_class_ctor, visited_arg_names, can_use_rest );
    }
  }

  std::string desc;
  Node::describe_tree_to_indented( *function, desc, 0 );
  workspace.report.debug( loc, "Generated function: {}", desc );
}

void GeneratedFunctionBuilder::add_base_constructor( std::unique_ptr<GeneratedFunction>& super,
                                                     UserFunction* base_class_ctor,
                                                     std::set<ScopableName>& visited_arg_names,
                                                     bool can_use_rest )
{
  auto& function_parameters = super->child<FunctionParameterList>( 0 ).children;
  auto& body = super->child<FunctionBody>( 1 ).children;
  auto params = base_class_ctor->parameters();
  const auto& loc = super->source_location;
  auto call_arguments = std::vector<std::unique_ptr<Argument>>();

  bool first = true;

  for ( auto& param_ref : params )
  {
    auto& param = param_ref.get();
    auto param_scope = first                      ? ScopeName::None
                       : param.name.scope.empty() ? ScopeName( base_class_ctor->name )
                                                  : param.name.scope;

    auto param_name = ScopableName( param_scope, param.name.name );


    // Skip the first `this` parameter of the function declaration, as we've already added it.
    if ( !first && visited_arg_names.find( param_name ) == visited_arg_names.end() )
    {
      // If the base ctor parameter is a rest param, our super() function parameter
      // will _not_ be a rest, but a regular variable with a default [empty]
      // array value. This only applies if we _cannot_ use rest parameters.
      if ( param.rest && !can_use_rest )
      {
        function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
            loc, param_name, param.byref, param.unused, false,
            std::make_unique<ArrayInitializer>( param.source_location,
                                                std::vector<std::unique_ptr<Expression>>() ) ) );
      }
      // If the base ctor parameter has a default value, our super() function
      // parameter will have the same default value.
      else if ( auto default_value = param.default_value() )
      {
        SimpleValueCloner cloner( report, default_value->source_location );

        // Value must be cloneable
        if ( auto final_argument = cloner.clone( *default_value ) )
        {
          function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
              loc, param_name, param.byref, false /* unused */, false /* rest */,
              std::move( final_argument ) ) );
        }
        else
        {
          report.error( loc,
                        "In construction of '{}': Unable to create argument from default for "
                        "parameter '{}'.\n"
                        "  See also: {}",
                        super->scoped_name(), param.name, param.source_location );
          return;
        }
      }
      else
      {
        // This super() alias'ed parameter is a rest parameter if the base
        // ctor's parameter is rest _and_ we can use a rest parameter.
        auto is_rest_param = param.rest && can_use_rest;

        function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
            loc, param_name, param.byref, false /* unused */, is_rest_param ) );
      }
    }

    // By default, the function call argument inside this super()'s function
    // declaration will just be the super() alias'ed parameter.
    std::unique_ptr<Expression> call_argument =
        std::make_unique<Identifier>( param.source_location, param_name );

    // If the base ctor parameter is a rest param, the base_ctor() function
    // call will spread the super() function parameter -- an array -- into
    // the base constructor.
    if ( param.rest )
    {
      // Passing an element that is not spreadable (eg. a string) will result
      // in an empty array passed to the base constructor.
      call_argument = std::make_unique<SpreadElement>( param.source_location,
                                                       std::move( call_argument ), false );
    }

    call_arguments.insert( call_arguments.end(),
                           std::make_unique<Argument>( param.source_location, param.name,
                                                       std::move( call_argument ), param.rest ) );
    visited_arg_names.insert( param_name );
    first = false;
  }

  auto fc = std::make_unique<FunctionCall>(
      loc, base_class_ctor->name, ScopableName( base_class_ctor->name, base_class_ctor->name ),
      std::move( call_arguments ) );

  fc->function_link->link_to( base_class_ctor );

  auto value_consumer = std::make_unique<ValueConsumer>( fc->source_location, std::move( fc ) );
  body.push_back( std::move( value_consumer ) );
}
}  // namespace Pol::Bscript::Compiler
