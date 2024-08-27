#include "GeneratedFunctionBuilder.h"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/sliced.hpp>
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
  std::vector<std::shared_ptr<ClassLink>> to_link;
  auto class_declaration = super->class_declaration();
  const auto& loc = class_declaration->source_location;

  to_link.insert( to_link.end(), class_declaration->base_class_links.begin(),
                  class_declaration->base_class_links.end() );

  while ( !to_link.empty() )
  {
    auto base_class_link = to_link.back();
    to_link.pop_back();
    if ( auto base_cd = base_class_link->class_declaration() )
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

  // If there are no base ctors, skip updating the function's parameters and
  // body, therefore a super function that is "invalid" will have no parameters
  // or body.
  if ( !base_class_ctors.empty() )
  {
    auto& function_parameters = super->child<FunctionParameterList>( 0 ).children;

    auto& body = super->child<FunctionBody>( 1 ).children;
    function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
        loc, ScopableName( ScopeName::None, "this" ), true /* byref */, false /* unused */,
        false /* rest */ ) );

    // Our super() alias'ed parameter can be a rest parameter only if the not-last
    // constructors are not variadic.
    bool can_use_rest = true;

    for ( const auto* base_class_ctor :
          base_class_ctors | boost::adaptors::reversed |
              boost::adaptors::sliced( 0, base_class_ctors.size() - 1 ) )
    {
      if ( base_class_ctor->is_variadic() )
      {
        can_use_rest = false;
        break;
      }
    }

    std::set<ScopableName> visited_arg_names;

    for ( auto base_class_ctor : boost::adaptors::reverse( base_class_ctors ) )
    {
      auto params = base_class_ctor->parameters();

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
                std::make_unique<ArrayInitializer>(
                    param.source_location, std::vector<std::unique_ptr<Expression>>() ) ) );
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
                  loc, param_name, param.byref, param.unused, false,
                  std::move( final_argument ) ) );
            }
            else
            {
              report.error( class_declaration->source_location,
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
                loc, param_name, param.byref, param.unused, is_rest_param ) );
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
          call_argument =
              std::make_unique<SpreadElement>( param.source_location, std::move( call_argument ) );
        }

        call_arguments.insert( call_arguments.end(), std::make_unique<Argument>(
                                                         param.source_location, param.name,
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

    body.push_back(
        std::make_unique<ReturnStatement>( loc, std::make_unique<Identifier>( loc, "this" ) ) );
  }

  std::string desc;
  Node::describe_tree_to_indented( *super, desc, 0 );
  workspace.report.debug( loc, "Super function: {}", desc );
}

void GeneratedFunctionBuilder::constructor_function(
    std::unique_ptr<GeneratedFunction>& constructor )
{
  std::set<ClassDeclaration*> visited;
  std::vector<std::shared_ptr<ClassLink>> to_visit;
  auto class_declaration = constructor->class_declaration();
  const auto& loc = class_declaration->source_location;
  UserFunction* base_class_ctor = nullptr;

  // If the class declaration _has_ a constructor link, then it registered a
  // constructor but it was never resolved. This should never happen.
  if ( class_declaration->constructor_link != nullptr )
  {
    // If the class has a _linked_ constructor, then we should not have
    // generated a constructor function.
    if ( class_declaration->constructor_link->function() != nullptr )
      class_declaration->internal_error(
          fmt::format( "class {} has linked ctor", class_declaration->name ) );

    class_declaration->internal_error(
        fmt::format( "class {} unlinked ctor", class_declaration->name ) );
  }

  to_visit.insert( to_visit.end(), class_declaration->base_class_links.begin(),
                   class_declaration->base_class_links.end() );

  while ( !to_visit.empty() )
  {
    auto base_class_link = to_visit.back();
    to_visit.pop_back();
    if ( auto base_cd = base_class_link->class_declaration() )
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
          to_visit.clear();
          break;
        }
      }
    }
  }

  // TODO remove duplicate code from `super_function`
  if ( base_class_ctor != nullptr )
  {
    auto& function_parameters = constructor->child<FunctionParameterList>( 0 ).children;
    auto& body = constructor->child<FunctionBody>( 1 ).children;
    auto call_arguments = std::vector<std::unique_ptr<Argument>>();
    bool first = true;

    for ( auto& param_ref : base_class_ctor->parameters() )
    {
      auto& param = param_ref.get();
      auto param_scope = first ? ScopeName::None : ScopeName( base_class_ctor->name );
      auto param_name = ScopableName( param_scope, param.name.name );

      // If the base ctor parameter has a default value, our generated function
      // parameter will have the same default value.
      if ( auto default_value = param.default_value() )
      {
        SimpleValueCloner cloner( report, default_value->source_location );

        // Value must be cloneable
        if ( auto final_argument = cloner.clone( *default_value ) )
        {
          function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
              loc, param_name, param.byref, param.unused, false, std::move( final_argument ) ) );
        }
        else
        {
          report.error( class_declaration->source_location,
                        "In construction of '{}': Unable to create argument from default for "
                        "parameter '{}'.\n"
                        "  See also: {}",
                        constructor->scoped_name(), param.name, param.source_location );
          return;
        }
      }
      else
      {
        function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
            loc, param_name, param.byref, param.unused, param.rest ) );
      }
      // The function call argument inside this generated function function
      // declaration will just be the generated function's aliased parameter.
      std::unique_ptr<Expression> call_argument =
          std::make_unique<Identifier>( param.source_location, param_name );

      call_arguments.insert( call_arguments.end(),
                             std::make_unique<Argument>( param.source_location,
                                                         std::move( call_argument ), param.rest ) );
      first = false;
    }
    // TODO update after Function implements ScopableName
    auto fc = std::make_unique<FunctionCall>( loc, base_class_ctor->name,
                                              ScopableName( constructor->name, constructor->name ),
                                              std::move( call_arguments ) );

    fc->function_link->link_to( base_class_ctor );

    auto value_consumer = std::make_unique<ValueConsumer>( fc->source_location, std::move( fc ) );
    body.push_back( std::move( value_consumer ) );
  }
  std::string desc;
  Node::describe_tree_to_indented( *constructor, desc, 0 );
  workspace.report.debug( loc, "Ctor {} function: {}", constructor->name, desc );
  class_declaration->constructor_link = std::make_unique<FunctionLink>( loc, "", true );
  class_declaration->constructor_link->link_to( constructor.get() );
}
}  // namespace Pol::Bscript::Compiler