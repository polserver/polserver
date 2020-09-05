#include "CaseStatement.h"

#include <format/format.h>
#include <utility>

#include "compiler/ast/CaseDispatchGroup.h"
#include "compiler/ast/CaseDispatchGroups.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/model/FlowControlLabel.h"

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

void CaseStatement::describe_to( fmt::Writer& w ) const
{
  w << "case-statement";
  if ( !get_label().empty() )
    w << "(label:" << get_label() << ")";
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
