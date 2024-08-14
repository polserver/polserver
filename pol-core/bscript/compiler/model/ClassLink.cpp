#include "ClassLink.h"

#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
ClassLink::ClassLink( const SourceLocation& source_location, std::string name )
    : source_location( source_location ),
      name( std::move( name ) ),
      linked_class_declaration( nullptr )
{
}

ClassDeclaration* ClassLink::class_declaration() const
{
  return linked_class_declaration;
}

void ClassLink::link_to( ClassDeclaration* f )
{
  if ( linked_class_declaration )
    source_location.internal_error( "class declaration already linked" );
  linked_class_declaration = f;
}

}  // namespace Pol::Bscript::Compiler
