#include "StructMemberInitializer.h"

#include <format/format.h>

#include "Expression.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
StructMemberInitializer::StructMemberInitializer( const SourceLocation& source_location,
                                                  std::string name )
    : Node( source_location ), name( std::move( name ) )
{
}

StructMemberInitializer::StructMemberInitializer( const SourceLocation& source_location,
                                                  std::string name,
                                                  std::unique_ptr<Expression> initializer )
    : Node( source_location, std::move( initializer ) ), name( std::move( name ) )
{
}

void StructMemberInitializer::accept( NodeVisitor& visitor )
{
  visitor.visit_struct_member_initializer( *this );
}

void StructMemberInitializer::describe_to( fmt::Writer& w ) const
{
  w << "struct-member-initializer(" << name << ")";
}

}  // namespace Pol::Bscript::Compiler
