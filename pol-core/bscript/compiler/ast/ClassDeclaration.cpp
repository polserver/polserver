#include "ClassDeclaration.h"

#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ClassDeclaration::ClassDeclaration( const SourceLocation& source_location, std::string name,
                                    std::unique_ptr<ClassParameterList> parameters,
                                    std::unique_ptr<ClassBody> body )
    : Node( source_location ), name( std::move( name ) )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
  children.push_back( std::move( body ) );
}

void ClassDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_class_declaration( *this );
}

void ClassDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-declaration({})", name );
}

}  // namespace Pol::Bscript::Compiler
