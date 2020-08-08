#include "NodeVisitor.h"

#include "Node.h"

namespace Pol::Bscript::Compiler
{
void NodeVisitor::visit_children( Node& parent )
{
  for ( const auto& child : parent.children )
  {
    child->accept( *this );
  }
}

}  // namespace Pol::Bscript::Compiler
