#include "ClassDeclaration.h"

#include "bscript/compiler/ast/ClassBody.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VarStatement.h"

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

std::vector<std::reference_wrapper<UserFunction>> ClassDeclaration::functions()
{
  std::vector<std::reference_wrapper<UserFunction>> functs;

  child<ClassBody>( 1 ).get_children<UserFunction>( functs );

  return functs;
}
std::vector<std::reference_wrapper<Identifier>> ClassDeclaration::parameters()
{
  std::vector<std::reference_wrapper<Identifier>> params;

  child<ClassParameterList>( 0 ).get_children<Identifier>( params );

  return params;
}

UserFunction* ClassDeclaration::constructor()
{
  for ( auto& f : functions() )
  {
    if ( f.get().type == UserFunctionType::Constructor )
    {
      return &f.get();
    }
  }
  return nullptr;
}
}  // namespace Pol::Bscript::Compiler
