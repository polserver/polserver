#include "CaseDispatchGroup.h"


#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/CaseDispatchSelectors.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

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

void CaseDispatchGroup::describe_to( std::string& w ) const
{
  w += type();
}

std::string CaseDispatchGroup::type() const
{
  return "case-dispatch-group";
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
