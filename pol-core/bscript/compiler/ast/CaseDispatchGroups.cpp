#include "CaseDispatchGroups.h"

#include <format/format.h>

#include "CaseDispatchGroup.h"
#include "NodeVisitor.h"

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

/*std::vector<std::shared_ptr<CaseDispatchGroup>> CaseDispatchGroups::dispatch_groups()
{
  std::vector<std::shared_ptr<CaseDispatchGroup>> groups;
  for ( auto& child : children )
  {
    groups.push_back( std::static_pointer_cast<CaseDispatchGroup>( child ) );
  }
  return groups;
}*/

}  // namespace Pol::Bscript::Compiler
