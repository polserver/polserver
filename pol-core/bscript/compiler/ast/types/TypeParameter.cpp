#include "TypeParameter.h"

#include "bscript/compiler/ast/types/TypeConstraint.h"
#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
TypeParameter::TypeParameter( const SourceLocation& source_location, std::string name )
    : Node( source_location ), name( name )
{
}

TypeParameter::TypeParameter( const SourceLocation& source_location, std::string name,
                              std::unique_ptr<TypeNode> default_value )
    : Node( source_location, std::move( default_value ) ), name( name )
{
}
TypeParameter::TypeParameter( const SourceLocation& source_location, std::string name,
                              std::unique_ptr<TypeConstraint> type_constraint )
    : Node( source_location, std::move( type_constraint ) ), name( name )
{
}

TypeParameter::TypeParameter( const SourceLocation& source_location, std::string name,
                              std::unique_ptr<TypeNode> default_value,
                              std::unique_ptr<TypeConstraint> type_constraint )
    : Node( source_location ), name( name )
{
  children.reserve( 2 );
  children.push_back( std::move( default_value ) );
  children.push_back( std::move( type_constraint ) );
}


void TypeParameter::accept( NodeVisitor& ) {}
void TypeParameter::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "type-parameter({})", name );
}

TypeNode* TypeParameter::default_value()
{
  if ( children.size() == 2 )
  {
    return &child<TypeNode>( 0 );
  }
  else if ( children.size() == 1 )
  {
    return dynamic_cast<TypeNode*>( children[0].get() );
  }

  return nullptr;
}

TypeConstraint* TypeParameter::type_constraint()
{
  if ( children.size() == 2 )
  {
    return &child<TypeConstraint>( 1 );
  }
  else if ( children.size() == 1 )
  {
    return dynamic_cast<TypeConstraint*>( children[0].get() );
  }

  return nullptr;
}
}  // namespace Pol::Bscript::Compiler
