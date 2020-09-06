#include "StructInitializer.h"

#include <format/format.h>
#include <utility>

#include "compiler/ast/NodeVisitor.h"
#include "compiler/ast/StructMemberInitializer.h"

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

void StructInitializer::describe_to( fmt::Writer& w ) const
{
  w << "struct-initializer";
}

}  // namespace Pol::Bscript::Compiler
