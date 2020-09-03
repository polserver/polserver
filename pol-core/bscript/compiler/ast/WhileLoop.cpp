#include "WhileLoop.h"

#include <format/format.h>

#include "compiler/ast/Block.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
WhileLoop::WhileLoop( const SourceLocation& source_location, std::string label,
                      std::unique_ptr<Expression> expression, std::unique_ptr<Block> block )
    : LoopStatement( source_location, std::move( label ) )
{
  children.reserve( 2 );
  children.push_back( std::move( expression ) );
  children.push_back( std::move( block ) );
}

void WhileLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_while_loop( *this );
}

void WhileLoop::describe_to( fmt::Writer& w ) const
{
  w << "while-loop";
  if ( !get_label().empty() )
    w << "(label:" << get_label() << ")";
}

Expression& WhileLoop::predicate()
{
  return child<Expression>( 0 );
}

Block& WhileLoop::block()
{
  return child<Block>( 1 );
}

}  // namespace Pol::Bscript::Compiler
