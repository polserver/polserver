#include "ClassDeclaration.h"


#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"

namespace Pol::Bscript::Compiler
{
ClassDeclaration::ClassDeclaration( const SourceLocation& source_location, std::string name,
                                    std::unique_ptr<ClassParameterList> parameters,
                                    std::vector<std::string> function_names, Node* class_body,
                                    std::shared_ptr<FunctionLink> constructor )
    : Node( source_location, std::move( parameters ) ),
      name( std::move( name ) ),
      function_names( std::move( function_names ) ),
      class_body( class_body ),
      constructor( std::move( constructor ) )
{
}

void ClassDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_class_declaration( *this );
}

void ClassDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-declaration({})", name );
}

std::vector<std::reference_wrapper<ClassParameterDeclaration>> ClassDeclaration::parameters()
{
  std::vector<std::reference_wrapper<ClassParameterDeclaration>> params;

  child<ClassParameterList>( 0 ).get_children<ClassParameterDeclaration>( params );

  return params;
}
}  // namespace Pol::Bscript::Compiler
