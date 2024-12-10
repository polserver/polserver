#include "MemberSignature.h"

namespace Pol::Bscript::Compiler
{
MemberSignature::MemberSignature( const SourceLocation& source_location ) : Node( source_location )
{
}

MemberSignature::MemberSignature( const SourceLocation& source_location, NodeVector children )
    : Node( source_location, std::move( children ) )
{
}

MemberSignature::MemberSignature( const SourceLocation& source_location,
                                  std::unique_ptr<Node> child )
    : Node( source_location, std::move( child ) )
{
}

void MemberSignature::accept( NodeVisitor& ) {}
}  // namespace Pol::Bscript::Compiler
