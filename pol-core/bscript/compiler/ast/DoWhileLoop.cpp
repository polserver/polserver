#include "DoWhileLoop.h"

#include <format/format.h>

#include "compiler/ast/Block.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
DoWhileLoop::DoWhileLoop( const SourceLocation& source_location, std::string label,
                          std::unique_ptr<Block> block, std::unique_ptr<Expression> expression )
    : LoopStatement( source_location, std::move( label ) )
{
  children.reserve( 2 );
  children.push_back( std::move( block ) );
  children.push_back( std::move( expression ) );
}

void DoWhileLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_do_while_loop( *this );
}

void DoWhileLoop::describe_to( fmt::Writer& w ) const
{
  w << "do-while-loop";
}

Block& DoWhileLoop::block()
{
  return child<Block>( 0 );
}

Expression& DoWhileLoop::predicate()
{
  return child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler
