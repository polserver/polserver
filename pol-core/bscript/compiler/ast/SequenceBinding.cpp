#include "SequenceBinding.h"

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "clib/clib.h"

namespace Pol::Bscript::Compiler
{
SequenceBinding::SequenceBinding( const SourceLocation& loc,
                                  std::vector<std::unique_ptr<Node>> bindings )
    : Node( loc, std::move( bindings ) )
{
}
u8 SequenceBinding::binding_count() const
{
  return Clib::clamp_convert<u8>( children.size() );
}

void SequenceBinding::accept( NodeVisitor& visitor )
{
  visitor.visit_sequence_binding( *this );
}

void SequenceBinding::describe_to( std::string& w ) const
{
  w += "sequence-binding";
}

}  // namespace Pol::Bscript::Compiler
