#include "BasicForLoop.h"

#include <format/format.h>
#include <utility>

#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
BasicForLoop::BasicForLoop( const SourceLocation& source_location, std::string label,
                            std::string identifier, std::unique_ptr<Expression> first,
                            std::unique_ptr<Expression> last, std::unique_ptr<Block> block )
    : LoopStatement( source_location, std::move( label ) ), identifier( std::move( identifier ) )
{
  children.reserve( 3 );
  children.push_back( std::move( first ) );
  children.push_back( std::move( last ) );
  children.push_back( std::move( block ) );
}

void BasicForLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_basic_for_loop( *this );
}

void BasicForLoop::describe_to( std::string& w ) const
{
  w += "basic-for-loop";
}

Expression& BasicForLoop::first()
{
  return child<Expression>( 0 );
}

Expression& BasicForLoop::last()
{
  return child<Expression>( 1 );
}

Block& BasicForLoop::block()
{
  return child<Block>( 2 );
}

}  // namespace Pol::Bscript::Compiler
