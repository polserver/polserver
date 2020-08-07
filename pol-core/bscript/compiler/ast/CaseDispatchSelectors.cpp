#include "CaseDispatchSelectors.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
CaseDispatchSelectors::CaseDispatchSelectors( const SourceLocation& source_location,
                                              NodeVector children )
    : Node( source_location, std::move( children ) )
{
}

void CaseDispatchSelectors::accept( NodeVisitor& visitor )
{
  visitor.visit_case_dispatch_selectors( *this );
}

void CaseDispatchSelectors::describe_to( fmt::Writer& w ) const
{
  w << "case-dispatch-selectors";
}

}  // namespace Pol::Bscript::Compiler
