#include "FunctionParameterDeclaration.h"

#include <utility>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
FunctionParameterDeclaration::FunctionParameterDeclaration(
    const SourceLocation& source_location, ScopableName name, bool byref, bool unused, bool rest,
    std::unique_ptr<Expression> default_value, std::unique_ptr<TypeNode> type )
    : Node( source_location ),
      name( std::move( name ) ),
      byref( byref ),
      unused( unused ),
      rest( rest )
{
  children.reserve( 2 );
  children.push_back( std::move( default_value ) );
  children.push_back( std::move( type ) );
}

FunctionParameterDeclaration::FunctionParameterDeclaration(
    const SourceLocation& source_location, ScopableName name, bool byref, bool unused, bool rest,
    std::unique_ptr<Expression> default_value )
    : Node( source_location, std::move( default_value ) ),
      name( std::move( name ) ),
      byref( byref ),
      unused( unused ),
      rest( rest )
{
}

FunctionParameterDeclaration::FunctionParameterDeclaration( const SourceLocation& source_location,
                                                            ScopableName name, bool byref,
                                                            bool unused, bool rest )
    : Node( source_location ),
      name( std::move( name ) ),
      byref( byref ),
      unused( unused ),
      rest( rest )
{
}

void FunctionParameterDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_function_parameter_declaration( *this );
}

void FunctionParameterDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "function-parameter-declaration({}", name.string() );
  if ( rest )
    w += "...";
  if ( byref )
    w += ", byref";
  if ( unused )
    w += ", unused";
  w += ")";
}

Expression* FunctionParameterDeclaration::default_value()
{
  return children.empty() ? nullptr : &child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
