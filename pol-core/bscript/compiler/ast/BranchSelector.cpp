#include "BranchSelector.h"


#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
BranchSelector::BranchSelector( const SourceLocation& source_location, BranchType branch_type,
                                std::unique_ptr<Expression> predicate )
    : Node( source_location, std::move( predicate ) ),
      branch_type( branch_type ),
      flow_control_label( std::make_shared<FlowControlLabel>() )
{
}

BranchSelector::BranchSelector( const SourceLocation& source_location, BranchType branch_type )
    : Node( source_location ),
      branch_type( branch_type ),
      flow_control_label( std::make_shared<FlowControlLabel>() )
{
}

void BranchSelector::accept( NodeVisitor& visitor )
{
  visitor.visit_branch_selector( *this );
}

void BranchSelector::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "branch-selector({})", fmt::underlying( branch_type ) );
}

Expression* BranchSelector::predicate()
{
  return children.empty() ? nullptr : &child<Expression>( 0 );
}

}  // namespace Pol::Bscript::Compiler
