#include "ForeachLoop.h"


#include <utility>

#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

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

void ForeachLoop::describe_to( std::string& w ) const
{
  w += "foreach-loop";
  if ( !get_label().empty() )
    fmt::format_to( std::back_inserter( w ), "(label:{})", get_label() );
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
