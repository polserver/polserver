#include "CaseDispatchDefaultSelector.h"


#include "bscript/compiler/ast/NodeVisitor.h"

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

void CaseDispatchDefaultSelector::describe_to( std::string& w ) const
{
  w += type();
}

std::string CaseDispatchDefaultSelector::type() const
{
  return "case-dispatch-default-selector";
}

}  // namespace Pol::Bscript::Compiler
