#include "CstyleForLoop.h"


#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
CstyleForLoop::CstyleForLoop( const SourceLocation& source_location, std::string label,
                              std::unique_ptr<Expression> initializer,
                              std::unique_ptr<Expression> predicate,
                              std::unique_ptr<Expression> advancer, std::unique_ptr<Block> block )
    : LoopStatement( source_location, std::move( label ) )
{
  children.reserve( 4 );
  children.push_back( std::move( initializer ) );
  children.push_back( std::move( predicate ) );
  children.push_back( std::move( advancer ) );
  children.push_back( std::move( block ) );
}

void CstyleForLoop::accept( NodeVisitor& visitor )
{
  visitor.visit_cstyle_for_loop( *this );
}

void CstyleForLoop::describe_to( std::string& w ) const
{
  w += "c-style-for-loop";
}

Expression& CstyleForLoop::initializer()
{
  return child<Expression>( 0 );
}

Expression& CstyleForLoop::predicate()
{
  return child<Expression>( 1 );
}

Expression& CstyleForLoop::advancer()
{
  return child<Expression>( 2 );
}

Block& CstyleForLoop::block()
{
  return child<Block>( 3 );
}


}  // namespace Pol::Bscript::Compiler
