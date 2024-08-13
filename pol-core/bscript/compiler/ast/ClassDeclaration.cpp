#include "ClassDeclaration.h"

#include <boost/range/adaptor/reversed.hpp>
#include <set>

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/ScopableName.h"

#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
ClassDeclaration::ClassDeclaration( const SourceLocation& source_location, std::string name,
                                    std::unique_ptr<ClassParameterList> parameters,
                                    std::vector<std::string> function_names, Node* class_body,
                                    std::vector<std::shared_ptr<ClassLink>> base_class_links )
    : Node( source_location, std::move( parameters ) ),
      name( std::move( name ) ),
      function_names( std::move( function_names ) ),
      class_body( class_body ),
      constructor_link(
          std::make_unique<FunctionLink>( source_location, name, true /* requires_ctor */ ) ),
      base_class_links( std::move( base_class_links ) )
{
}

void ClassDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_class_declaration( *this );
}

void ClassDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-declaration({})", name );
}

std::vector<std::reference_wrapper<ClassParameterDeclaration>> ClassDeclaration::parameters()
{
  std::vector<std::reference_wrapper<ClassParameterDeclaration>> params;

  child<ClassParameterList>( 0 ).get_children<ClassParameterDeclaration>( params );

  return params;
}

// TODO move this to a "builder"
UserFunction* ClassDeclaration::make_super( const SourceLocation& loc )
{
  if ( constructor_link->user_function() == nullptr )
  {
    return nullptr;
  }
  else if ( super )
  {
    return super.get();
  }

  std::vector<UserFunction*> base_class_ctors;
  std::set<ClassDeclaration*> visited;
  std::vector<std::shared_ptr<ClassLink>> to_link;
  to_link.insert( to_link.end(), base_class_links.begin(), base_class_links.end() );

  auto super = std::make_unique<UserFunction>(
      loc, false, false, UserFunctionType::Super, name /* scope */, "super" /* function name */,
      std::make_unique<FunctionParameterList>(
          loc, std::vector<std::unique_ptr<FunctionParameterDeclaration>>() ),
      std::make_unique<FunctionBody>( loc, std::vector<std::unique_ptr<Statement>>() ), loc,
      nullptr );

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
        // If base class no constructor, we cannot make a super().
        return nullptr;
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
        loc, name, ScopableName( base_class_ctor->name, base_class_ctor->name ),
        std::move( call_arguments ) );

    fc->function_link->link_to( base_class_ctor );

    auto value_consumer = std::make_unique<ValueConsumer>( fc->source_location, std::move( fc ) );
    body.push_back( std::move( value_consumer ) );
  }

  body.push_back(
      std::make_unique<ReturnStatement>( loc, std::make_unique<Identifier>( loc, "this" ) ) );

  std::string desc;
  Node::describe_tree_to_indented( *super, desc, 0 );

  INFO_PRINTLN( "Super function: {}", desc );

  this->super = std::move( super );

  return this->super.get();
}

std::unique_ptr<UserFunction> ClassDeclaration::take_super()
{
  return std::move( super );
}

}  // namespace Pol::Bscript::Compiler
