#include "ThisMemberAssignmentGatherer.h"

#include "bscript/compiler/ast/FunctionExpression.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
void ThisMemberAssignmentGatherer::visit_member_assignment( MemberAssignment& node )
{
  if ( node.children.empty() )
    return;

  auto& expression = node.entity();
  if ( auto identifier = dynamic_cast<Identifier*>( &expression ) )
  {
    if ( Clib::caseInsensitiveEqual( identifier->scoped_name.string(), "this" ) )
    {
      assignments.push_back( &node );
    }
  }
}

void ThisMemberAssignmentGatherer::visit_function_expression( FunctionExpression& node )
{
  // The body is not a child of the FunctionExpression: it lives in the linked UserFunction.
  auto user_function = node.function_link ? node.function_link->user_function() : nullptr;
  if ( !user_function )
    return;

  // A `this` parameter shadows the class instance instead of capturing it.
  for ( const auto& param : user_function->parameters() )
  {
    if ( Clib::caseInsensitiveEqual( param.get().name.string(), "this" ) )
      return;
  }

  user_function->accept( *this );
}
}  // namespace Pol::Bscript::Compiler