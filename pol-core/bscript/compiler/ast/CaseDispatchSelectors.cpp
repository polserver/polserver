#include "CaseDispatchSelectors.h"


#include "bscript/compiler/ast/NodeVisitor.h"

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

void CaseDispatchSelectors::describe_to( std::string& w ) const
{
  w += type();
}

std::string CaseDispatchSelectors::type() const
{
  return "case-dispatch-selectors";
}

}  // namespace Pol::Bscript::Compiler
