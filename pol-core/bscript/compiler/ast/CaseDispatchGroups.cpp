#include "CaseDispatchGroups.h"

#include <format/format.h>

#include "compiler/ast/CaseDispatchGroup.h"
#include "compiler/ast/NodeVisitor.h"

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

void CaseDispatchGroups::describe_to( fmt::Writer& w ) const
{
  w << "case-dispatch-groups";
}

CaseDispatchGroup& CaseDispatchGroups::dispatch_group( int index )
{
  return child<CaseDispatchGroup>( index );
}

}  // namespace Pol::Bscript::Compiler
