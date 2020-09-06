#include "ForeachLoop.h"

#include <format/format.h>
#include <utility>

#include "compiler/ast/Block.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ForeachLoop::ForeachLoop( const SourceLocation& source_location, std::string label,
                          std::string iterator_name, std::unique_ptr<Expression> expression,
                          std::unique_ptr<Block> block )
    : LoopStatement( source_location, std::move( label ) ),
      iterator_name( std::move( iterator_name ) )
{
  children.reserve( 2 );
  children.push_back( std::move( expression ) );
  children.push_back( std::move( block ) );
}

void ForeachLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_foreach_loop( *this );
}

void ForeachLoop::describe_to( fmt::Writer& w ) const
{
  w << "foreach-loop";
  if ( !get_label().empty() )
    w << "(label:" << get_label() << ")";
}

Expression& ForeachLoop::expression()
{
  return child<Expression>( 0 );
}

Block& ForeachLoop::block()
{
  return child<Block>( 1 );
}

}  // namespace Pol::Bscript::Compiler
