#include "ConditionalExpression.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/FlowControlLabel.h"

namespace Pol::Bscript::Compiler
{
ConditionalExpression::ConditionalExpression( const SourceLocation& source_location,
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

void ConditionalExpression::accept( NodeVisitor& visitor )
{
  visitor.visit_conditional_expression( *this );
}

void ConditionalExpression::describe_to( fmt::Writer& w ) const
{
  w << "conditional-expression";
}

Expression& ConditionalExpression::conditional()
{
  return child<Expression>( 0 );
}

Expression& ConditionalExpression::consequent()
{
  return child<Expression>( 1 );
}

Expression& ConditionalExpression::alternate()
{
  return child<Expression>( 2 );
}

}  // namespace Pol::Bscript::Compiler
