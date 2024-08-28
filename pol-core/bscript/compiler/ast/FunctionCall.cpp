#include "FunctionCall.h"


#include <map>
#include <utility>

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
FunctionCall::FunctionCall( const SourceLocation& source_location, std::string calling_scope,
                            ScopableName scoped_name,
                            std::vector<std::unique_ptr<Argument>> arguments )
    : Expression( source_location, std::move( arguments ) ),
      function_link( std::make_shared<FunctionLink>( source_location, calling_scope ) ),
      calling_scope( std::move( calling_scope ) ),
      scoped_name( std::make_unique<ScopableName>( std::move( scoped_name ) ) )
{
}

FunctionCall::FunctionCall( const SourceLocation& source_location, std::string calling_scope,
                            std::unique_ptr<Node> callee,
                            std::vector<std::unique_ptr<Argument>> arguments )
    : Expression( source_location, std::move( arguments ) ),
      function_link( std::make_shared<FunctionLink>( source_location, calling_scope ) ),
      calling_scope( std::move( calling_scope ) ),
      scoped_name( nullptr )  // FunctionCalls with expression-as-callees do not have a scoped name
{
  children.insert( children.begin(), std::move( callee ) );
}

void FunctionCall::accept( NodeVisitor& visitor )
{
  visitor.visit_function_call( *this );
}

void FunctionCall::describe_to( std::string& w ) const
{
  if ( scoped_name )
  {
    fmt::format_to( std::back_inserter( w ), "function-call({})", scoped_name->string() );
  }
  else
  {
    fmt::format_to( std::back_inserter( w ), "expression-as-callee function-call" );
  }
}

std::vector<std::unique_ptr<Argument>> FunctionCall::take_arguments()
{
  std::vector<std::unique_ptr<Argument>> args;
  args.reserve( children.size() );
  for ( auto& child : children )
  {
    args.emplace_back( static_unique_pointer_cast<Argument, Node>( std::move( child ) ) );
  }
  return args;
}

std::vector<std::reference_wrapper<FunctionParameterDeclaration>> FunctionCall::parameters() const
{
  if ( auto fn = function_link->function() )
    return fn->parameters();
  else
    internal_error( "function has not been resolved" );
}

std::string FunctionCall::string() const
{
  // Should only be used on compile-time function calls (ie. not an
  // expression-as-callee function call) that actually have a ScopedName.
  if ( scoped_name )
    return scoped_name->string();
  return "";
}
}  // namespace Pol::Bscript::Compiler
