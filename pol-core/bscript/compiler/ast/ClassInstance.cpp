#include "ClassInstance.h"

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ClassInstance::ClassInstance( const SourceLocation& source_location,
                              ClassDeclaration* class_declaration )
    : Expression( source_location ), class_declaration( class_declaration )
{
}

void ClassInstance::accept( NodeVisitor& visitor )
{
  visitor.visit_class_instance( *this );
}

void ClassInstance::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-instance" );
}

}  // namespace Pol::Bscript::Compiler
