#include "ClassDeclaration.h"


#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"

namespace Pol::Bscript::Compiler
{
ClassDeclaration::ClassDeclaration( const SourceLocation& source_location, std::string name,
                                    std::unique_ptr<ClassParameterList> parameters,
                                    std::vector<std::string> function_names, Node* class_body )
    : Node( source_location ),
      name( std::move( name ) ),
      function_names( std::move( function_names ) ),
      class_body( class_body )
{
  children.reserve( 2 );
  children.push_back( std::move( parameters ) );
}

void ClassDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_class_declaration( *this );
}

void ClassDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-declaration({})", name );
}

std::vector<std::reference_wrapper<Identifier>> ClassDeclaration::parameters()
{
  std::vector<std::reference_wrapper<Identifier>> params;

  child<ClassParameterList>( 0 ).get_children<Identifier>( params );

  return params;
}
}  // namespace Pol::Bscript::Compiler
