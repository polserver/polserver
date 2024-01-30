#include "StructInitializer.h"


#include <utility>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/StructMemberInitializer.h"

namespace Pol::Bscript::Compiler
{
StructInitializer::StructInitializer(
    const SourceLocation& source_location,
    std::vector<std::unique_ptr<StructMemberInitializer>> initializers )
    : Expression( source_location, std::move( initializers ) )
{
}

void StructInitializer::accept( NodeVisitor& visitor )
{
  visitor.visit_struct_initializer( *this );
}

void StructInitializer::describe_to( std::string& w ) const
{
  w += type();
}

std::string StructInitializer::type() const
{
  return "struct-initializer";
}

}  // namespace Pol::Bscript::Compiler
