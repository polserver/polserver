#include "IfThenElseStatement.h"

#include <format/format.h>

#include "compiler/ast/Block.h"
#include "compiler/ast/BranchSelector.h"
#include "compiler/ast/Expression.h"
#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
IfThenElseStatement::IfThenElseStatement( const SourceLocation& source_location,
                                          std::unique_ptr<BranchSelector> branch_selector,
                                          std::unique_ptr<Block> consequent,
                                          std::unique_ptr<Node> alternative )
    : Statement( source_location )
{
  children.reserve( 3 );
  children.push_back( std::move( branch_selector ) );
  children.push_back( std::move( consequent ) );
  children.push_back( std::move( alternative ) );
}

IfThenElseStatement::IfThenElseStatement( const SourceLocation& source_location,
                                          std::unique_ptr<BranchSelector> branch_selector,
                                          std::unique_ptr<Block> consequent )
    : Statement( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( branch_selector ) );
  children.push_back( std::move( consequent ) );
}

void IfThenElseStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_if_then_else_statement( *this );
}

void IfThenElseStatement::describe_to( fmt::Writer& w ) const
{
  w << "if-then-else-statement";
}

BranchSelector& IfThenElseStatement::branch_selector()
{
  return child<BranchSelector>( 0 );
}

Block& IfThenElseStatement::consequent()
{
  return child<Block>( 1 );
}

Node* IfThenElseStatement::alternative()
{
  return ( children.size() >= 3 ) ? children.at( 2 ).get() : nullptr;
}

std::unique_ptr<Block> IfThenElseStatement::take_consequent()
{
  return take_child<Block>( 1 );
}

std::unique_ptr<Node> IfThenElseStatement::take_alternative()
{
  if ( children.size() >= 3 )
    return std::move( children[2] );
  else
    return {};
}

}  // namespace Pol::Bscript::Compiler
