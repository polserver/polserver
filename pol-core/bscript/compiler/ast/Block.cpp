#include "Block.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
Block::Block( const SourceLocation& source_location,
              std::vector<std::unique_ptr<Statement>> statements )
    : Statement( source_location )
{
  children.reserve( statements.size() );
  for ( auto& statement : statements )
  {
    children.push_back( std::move( statement ) );
  }
}

void Block::accept( NodeVisitor& visitor )
{
  visitor.visit_block( *this );
}

void Block::describe_to( std::string& w ) const
{
  w += "block";
}

}  // namespace Pol::Bscript::Compiler
