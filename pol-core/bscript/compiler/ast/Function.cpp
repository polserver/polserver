#include "Function.h"

#include "FunctionBody.h"
#include "FunctionParameterDeclaration.h"
#include "FunctionParameterList.h"

namespace Pol::Bscript::Compiler
{
Function::Function( const SourceLocation& source_location, std::string name,
                    std::unique_ptr<FunctionParameterList> parameter_list,
                    std::unique_ptr<FunctionBody> body )
    : Node( source_location ), name( std::move( name ) )
{
  children.reserve( 2 );
  children.push_back( std::move( parameter_list ) );
  children.push_back( std::move( body ) );
}

Function::Function( const SourceLocation& source_location, std::string name,
                    std::unique_ptr<FunctionParameterList> parameter_list )
    : Node( source_location, std::move( parameter_list ) ), name( std::move( name ) )
{
}

unsigned Function::parameter_count() const
{
  return static_cast<unsigned>(children.at( 0 )->children.size());
}

std::vector<std::reference_wrapper<FunctionParameterDeclaration>> Function::parameters()
{
  std::vector<std::reference_wrapper<FunctionParameterDeclaration>> params;
  auto& param_list = child<FunctionParameterList>( 0 );
  for ( auto& param : param_list.children )
  {
    params.emplace_back( *static_cast<FunctionParameterDeclaration*>( param.get() ) );
  }
  return params;
}

}  // namespace Pol::Bscript::Compiler