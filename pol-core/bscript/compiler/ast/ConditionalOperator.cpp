#include "ConditionalOperator.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
ConditionalOperator::ConditionalOperator( const SourceLocation& source_location,
                                          std::unique_ptr<Expression> conditional,
                                          std::unique_ptr<Expression> consequent,
                                          std::unique_ptr<Expression> alternate )
    : Expression( source_location ),
      consequent_label( std::make_shared<FlowControlLabel>() ),
      alternate_label( std::make_shared<FlowControlLabel>() )
{
  children.reserve( 3 );

  children.push_back( std::move( conditional ) );
  children.push_back( std::move( consequent ) );
  children.push_back( std::move( alternate ) );
}

void ConditionalOperator::accept( NodeVisitor& visitor )
{
  visitor.visit_conditional_operator( *this );
}

void ConditionalOperator::describe_to( std::string& w ) const
{
  w += "conditional-expression";
}

Expression& ConditionalOperator::conditional()
{
  return child<Expression>( 0 );
}

Expression& ConditionalOperator::consequent()
{
  return child<Expression>( 1 );
}

Expression& ConditionalOperator::alternate()
{
  return child<Expression>( 2 );
}

}  // namespace Pol::Bscript::Compiler
