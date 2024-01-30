#include "CaseDispatchGroups.h"


#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
CaseDispatchGroups::CaseDispatchGroups(
    const SourceLocation& context, std::vector<std::unique_ptr<CaseDispatchGroup>> dispatch_groups )
    : Node( context, std::move( dispatch_groups ) )
{
}

void CaseDispatchGroups::accept( NodeVisitor& visitor )
{
  visitor.visit_case_dispatch_groups( *this );
}

void CaseDispatchGroups::describe_to( std::string& w ) const
{
  w += type();
}

std::string CaseDispatchGroups::type() const
{
  return "case-dispatch-groups";
}

CaseDispatchGroup& CaseDispatchGroups::dispatch_group( size_t index )
{
  return child<CaseDispatchGroup>( index );
}

}  // namespace Pol::Bscript::Compiler
