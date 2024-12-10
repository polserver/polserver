#include "Function.h"

#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"

namespace Pol::Bscript::Compiler
{
Function::Function( const SourceLocation& source_location, std::string scope, std::string name,
                    std::unique_ptr<TypeParameterList> type_parameters,
                    std::unique_ptr<FunctionParameterList> parameter_list,
                    std::unique_ptr<FunctionBody> body, std::unique_ptr<TypeNode> type_annotation )
    : Node( source_location ), scope( std::move( scope ) ), name( std::move( name ) )
{
  if ( type_annotation )
  {
    children.reserve( 4 );
    children.push_back( std::move( parameter_list ) );
    children.push_back( std::move( body ) );
    children.push_back( std::move( type_parameters ) );
    children.push_back( std::move( type_annotation ) );
  }
  else
  {
    children.reserve( 3 );
    children.push_back( std::move( parameter_list ) );
    children.push_back( std::move( body ) );
    children.push_back( std::move( type_parameters ) );
  }
}

Function::Function( const SourceLocation& source_location, std::string scope, std::string name,
                    std::unique_ptr<TypeParameterList> type_parameters,
                    std::unique_ptr<FunctionParameterList> parameter_list,
                    std::unique_ptr<TypeNode> type_annotation )
    : Node( source_location ), scope( std::move( scope ) ), name( std::move( name ) )
{
  if ( type_annotation )
  {
    children.reserve( 3 );
    children.push_back( std::move( parameter_list ) );
    children.push_back( std::move( type_parameters ) );
    children.push_back( std::move( type_annotation ) );
  }
  else
  {
    children.reserve( 2 );
    children.push_back( std::move( parameter_list ) );
    children.push_back( std::move( type_parameters ) );
  }
}

unsigned Function::parameter_count() const
{
  return static_cast<unsigned>( children.at( 0 )->children.size() );
}

bool Function::is_variadic() const
{
  // The variadic argument, if present, is the last one
  if ( parameter_count() > 0 )
  {
    const auto& parameter_list = children.at( 0 );

    auto parameter =
        static_cast<FunctionParameterDeclaration*>( parameter_list->children.back().get() );

    return parameter->rest;
  }

  return false;
}

std::string Function::scoped_name() const
{
  if ( scope.empty() )
  {
    return name;
  }

  return fmt::format( "{}::{}", scope, name );
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