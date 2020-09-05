#include "CaseDispatchGroup.h"

#include <format/format.h>

#include "compiler/ast/Block.h"
#include "compiler/ast/CaseDispatchSelectors.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
CaseDispatchGroup::CaseDispatchGroup( const SourceLocation& source_location,
                                      std::unique_ptr<CaseDispatchSelectors> selectors,
                                      std::unique_ptr<Block> block )
    : Node( source_location ), break_label( std::make_shared<FlowControlLabel>() )
{
  children.reserve( 2 );
  children.push_back( std::move( selectors ) );
  children.push_back( std::move( block ) );
}

void CaseDispatchGroup::accept( NodeVisitor& visitor )
{
  visitor.visit_case_dispatch_group( *this );
}

void CaseDispatchGroup::describe_to( fmt::Writer& w ) const
{
  w << "case-dispatch-group";
}

CaseDispatchSelectors& CaseDispatchGroup::selectors()
{
  return child<CaseDispatchSelectors>( 0 );
}

Block& CaseDispatchGroup::block()
{
  return child<Block>( 1 );
}

}  // namespace Pol::Bscript::Compiler
