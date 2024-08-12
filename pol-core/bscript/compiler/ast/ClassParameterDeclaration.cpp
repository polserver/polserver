#include "ClassParameterDeclaration.h"

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
ClassParameterDeclaration::ClassParameterDeclaration( const SourceLocation& source_location,
                                                      std::string name )
    : Node( source_location ),
      name( std::move( name ) ),
      constructor_link( std::make_shared<FunctionLink>( source_location, "" /* calling scope */ ) )
{
}

void ClassParameterDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_class_parameter_declaration( *this );
}

void ClassParameterDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-parameter-declaration({})", name );
}
}  // namespace Pol::Bscript::Compiler
