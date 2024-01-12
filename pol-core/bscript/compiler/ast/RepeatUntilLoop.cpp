#include "RepeatUntilLoop.h"

#include <format/format.h>

#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
RepeatUntilLoop::RepeatUntilLoop( const SourceLocation& source_location, std::string label,
                                  std::unique_ptr<Block> block,
                                  std::unique_ptr<Expression> expression )
    : LoopStatement( source_location, std::move( label ) )
{
  children.reserve( 2 );
  children.push_back( std::move( block ) );
  children.push_back( std::move( expression ) );
}

void RepeatUntilLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_repeat_until_loop( *this );
}

void RepeatUntilLoop::describe_to( std::string& w ) const
{
  w += "repeat-until-loop";
  if ( !get_label().empty() )
    fmt::format_to( std::back_inserter( w ), "(label:{})", get_label() );
}

Block& RepeatUntilLoop::block()
{
  return child<Block>( 0 );
}

Expression& RepeatUntilLoop::expression()
{
  return child<Expression>( 1 );
}

}  // namespace Pol::Bscript::Compiler
