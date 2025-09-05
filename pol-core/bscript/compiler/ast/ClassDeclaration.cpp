#include "ClassDeclaration.h"

#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/ClassParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/UninitializedFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/FunctionLink.h"

#include <ranges>

namespace Pol::Bscript::Compiler
{
ClassDeclaration::ClassDeclaration(
    const SourceLocation& source_location, std::string name,
    std::unique_ptr<ClassParameterList> parameters, std::shared_ptr<FunctionLink> constructor_link,
    const std::vector<std::string>& method_names, Node* class_body,
    std::vector<std::shared_ptr<ClassLink>> base_class_links,
    std::vector<std::unique_ptr<UninitializedFunctionDeclaration>> uninit_functions )
    : Node( source_location ),
      name( std::move( name ) ),
      class_body( class_body ),
      constructor_link( std::move( constructor_link ) ),
      base_class_links( std::move( base_class_links ) )
{
  children.reserve( 1 + uninit_functions.size() );
  children.push_back( std::move( parameters ) );
  std::ranges::move( uninit_functions, std::back_inserter( children ) );
  for ( const auto& method_name : method_names )
  {
    methods[method_name] = std::make_unique<FunctionLink>( source_location, method_name );
  }
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

std::vector<std::reference_wrapper<UninitializedFunctionDeclaration>>
ClassDeclaration::uninit_functions()
{
  std::vector<std::reference_wrapper<UninitializedFunctionDeclaration>> params;

  // The first child is a `ClassParameterList`, so skip it.
  for ( auto& param : children | std::views::drop( 1 ) )
  {
    params.emplace_back( *static_cast<UninitializedFunctionDeclaration*>( param.get() ) );
  }

  return params;
}

std::string ClassDeclaration::type_tag() const
{
  return fmt::format( "{}@{}", name, source_location.source_file_identifier->pathname );
}
}  // namespace Pol::Bscript::Compiler
