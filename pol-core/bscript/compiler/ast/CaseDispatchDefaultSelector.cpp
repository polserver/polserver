#include "CaseDispatchDefaultSelector.h"

#include <format/format.h>

#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
CaseDispatchDefaultSelector::CaseDispatchDefaultSelector( const SourceLocation& source_location )
    : Node( source_location )
{
}

void CaseDispatchDefaultSelector::accept( NodeVisitor& visitor )
{
  visitor.visit_case_dispatch_default_selector( *this );
}

void CaseDispatchDefaultSelector::describe_to( fmt::Writer& w ) const
{
  w << "case-dispatch-default-selector";
}

}  // namespace Pol::Bscript::Compiler