#include "NodeVisitor.h"

#include "Node.h"
#include "TopLevelStatements.h"
#include "ValueConsumer.h"

namespace Pol::Bscript::Compiler
{

void NodeVisitor::visit_top_level_statements( TopLevelStatements& node )
{
  visit_children( node );
}

void NodeVisitor::visit_value_consumer( ValueConsumer& node )
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
