#include "FunctionCall.h"


#include <map>
#include <utility>

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/FunctionLink.h"

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

void FunctionCall::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "function-call({})", method_name );
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

std::unique_ptr<std::vector<std::reference_wrapper<FunctionParameterDeclaration>>>
FunctionCall::parameters() const
{
  if ( auto fn = function_link->function() )
    return std::make_unique<std::vector<std::reference_wrapper<FunctionParameterDeclaration>>>(
        fn->parameters() );
  else
    return {};
}

}  // namespace Pol::Bscript::Compiler
