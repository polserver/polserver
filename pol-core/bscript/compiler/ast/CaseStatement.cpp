#include "CaseStatement.h"


#include <utility>

#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/CaseDispatchGroups.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
class CaseDispatchGroups;

CaseStatement::CaseStatement( const SourceLocation& source_location, std::string label,
                              std::unique_ptr<Expression> expression,
                              std::unique_ptr<CaseDispatchGroups> dispatch_groups )
    : LabelableStatement( source_location, std::move( label ) ),
      break_label( std::make_shared<FlowControlLabel>() )
{
  children.reserve( 2 );
  children.push_back( std::move( expression ) );
  children.push_back( std::move( dispatch_groups ) );
}

void CaseStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_case_statement( *this );
}

void CaseStatement::describe_to( std::string& w ) const
{
  w += "case-statement";
  if ( !get_label().empty() )
    fmt::format_to( std::back_inserter( w ), "(label:{})", get_label() );
}

Expression& CaseStatement::expression()
{
  return child<Expression>( 0 );
}

CaseDispatchGroups& CaseStatement::dispatch_groups()
{
  return child<CaseDispatchGroups>( 1 );
}

}  // namespace Pol::Bscript::Compiler
