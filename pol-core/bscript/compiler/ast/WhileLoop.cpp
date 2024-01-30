#include "WhileLoop.h"


#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

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

void WhileLoop::describe_to( std::string& w ) const
{
  w += type();
  if ( !get_label().empty() )
    fmt::format_to( std::back_inserter( w ), "(label:{})", get_label() );
}

std::string WhileLoop::type() const
{
  return "while-loop";
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
