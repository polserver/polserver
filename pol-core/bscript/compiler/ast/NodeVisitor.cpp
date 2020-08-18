#include "NodeVisitor.h"

#include "compiler/ast/Node.h"
#include "compiler/ast/TopLevelStatements.h"

namespace Pol::Bscript::Compiler
{

void NodeVisitor::visit_top_level_statements( TopLevelStatements& node )
{
  visit_children( node );
}

void NodeVisitor::visit_children( Node& parent )
{
  for ( const auto& child : parent.children )
  {
    child->accept( *this );
  }
}

}  // namespace Pol::Bscript::Compiler
