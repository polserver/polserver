#include "FunctionCall.h"

#include <format/format.h>
#include <map>
#include <utility>

#include "Argument.h"
#include "FunctionParameterDeclaration.h"
#include "ModuleFunctionDeclaration.h"
#include "NodeVisitor.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
FunctionCall::FunctionCall( const SourceLocation& source_location, std::string scope,
                            std::string name, std::vector<std::unique_ptr<Argument>> children )
    : Expression( source_location, std::move( children ) ),
      function_link( std::make_shared<FunctionLink>( source_location ) ),
      scope( std::move( scope ) ),
      method_name( std::move( name ) )
{
}

void FunctionCall::accept( NodeVisitor& visitor )
{
  visitor.visit_function_call( *this );
}

void FunctionCall::describe_to( fmt::Writer& w ) const
{
  w << "function-call(" << method_name << ")";
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

}  // namespace Pol::Bscript::Compiler
