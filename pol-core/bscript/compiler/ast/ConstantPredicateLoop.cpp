#include "ConstantPredicateLoop.h"


#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ConstantPredicateLoop::ConstantPredicateLoop( const SourceLocation& source_location,
                                              std::string label, std::unique_ptr<Block> block,
                                              bool endless )
    : LoopStatement( source_location, std::move( label ) ), _endless( endless )
{
  children.reserve( 1 );
  children.push_back( std::move( block ) );
}

void ConstantPredicateLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_constant_loop( *this );
}

void ConstantPredicateLoop::describe_to( std::string& w ) const
{
  w += "constant-loop";
  if ( !get_label().empty() )
    fmt::format_to( std::back_inserter( w ), "(label:{})", get_label() );
}

Block& ConstantPredicateLoop::block()
{
  return child<Block>( 0 );
}

bool ConstantPredicateLoop::is_endless() const
{
  return _endless;
}
}  // namespace Pol::Bscript::Compiler
