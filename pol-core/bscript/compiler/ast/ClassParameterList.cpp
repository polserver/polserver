#include "ClassParameterList.h"

#include "bscript/compiler/ast/ClassParameterDeclaration.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ClassParameterList::ClassParameterList(
    const SourceLocation& source_location,
    std::vector<std::unique_ptr<ClassParameterDeclaration>> parameters )
    : Node( source_location, std::move( parameters ) )
{
}

void ClassParameterList::accept( NodeVisitor& visitor )
{
  visitor.visit_class_parameter_list( *this );
}

void ClassParameterList::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "class-parameter-list" );
}
}  // namespace Pol::Bscript::Compiler
