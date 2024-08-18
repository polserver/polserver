#include "GeneratedFunctionBuilder.h"

#include <boost/range/adaptor/reversed.hpp>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/SuperFunction.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
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
bool GeneratedFunctionBuilder::super_function( std::unique_ptr<SuperFunction>& super )
{
  std::vector<UserFunction*> base_class_ctors;
  std::set<ClassDeclaration*> visited;
  std::vector<std::shared_ptr<ClassLink>> to_link;
  auto class_declaration = super->class_declaration;
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

      if ( auto base_class_ctor = base_cd->constructor_link->user_function() )
      {
        base_class_ctors.push_back( base_class_ctor );
      }
      else
      {
        // If base class no constructor, we cannot make a super(). Some semantic
        // errors will happen further down the compilation line.
        return false;
      }
    }
  }

  auto& function_parameters = super->child<FunctionParameterList>( 0 ).children;

  auto& body = super->child<FunctionBody>( 1 ).children;
  function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
      loc, ScopableName( ScopeName::None, "this" ), true /* byref */, false /* unused */,
      false /* rest */ ) );

  for ( auto base_class_ctor : boost::adaptors::reverse( base_class_ctors ) )
  {
    auto params = base_class_ctor->parameters();

    auto call_arguments = std::vector<std::unique_ptr<Argument>>();

    bool first = true;

    for ( auto& param_ref : params )
    {
      auto& param = param_ref.get();

      // Skip the first `this` parameter of the function declaration, as we've already added it.
      if ( !first )
      {
        // TODO how does this work with duplicate names across multiple base classes
        function_parameters.push_back( std::make_unique<FunctionParameterDeclaration>(
            loc, ScopableName( base_class_ctor->name, param.name.name ), param.byref, param.unused,
            param.rest ) );
      }
      call_arguments.insert(
          call_arguments.end(),
          std::make_unique<Argument>(
              param.source_location, param.name,
              std::make_unique<Identifier>(
                  param.source_location,
                  ScopableName( first ? ScopeName::None : ScopeName( base_class_ctor->name ),
                                param.name.name ) ),
              false ) );

      first = false;
    }

    auto fc = std::make_unique<FunctionCall>(
        loc, "name", ScopableName( base_class_ctor->name, base_class_ctor->name ),
        std::move( call_arguments ) );

    fc->function_link->link_to( base_class_ctor );

    auto value_consumer = std::make_unique<ValueConsumer>( fc->source_location, std::move( fc ) );
    body.push_back( std::move( value_consumer ) );
  }

  body.push_back(
      std::make_unique<ReturnStatement>( loc, std::make_unique<Identifier>( loc, "this" ) ) );

  std::string desc;
  Node::describe_tree_to_indented( *super, desc, 0 );
  workspace.report.debug( loc, "Super function: {}", desc );

  return true;
}
}  // namespace Pol::Bscript::Compiler