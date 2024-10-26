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
ThisMemberAssignmentGatherer::ThisMemberAssignmentGatherer() {}

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
  // Do not visit children if encountering a FunctionExpression that has shadows
  // the `this` parameter instead of capturing it.
  if ( auto function_link = node.function_link )
  {
    if ( auto user_function = function_link->user_function() )
    {
      for ( const auto param : user_function->parameters() )
      {
        if ( Clib::caseInsensitiveEqual( param.get().name.string(), "this" ) )
        {
          return;
        }
      }
    }
  }

visit_children( node );
}
}  // namespace Pol::Bscript::Compiler