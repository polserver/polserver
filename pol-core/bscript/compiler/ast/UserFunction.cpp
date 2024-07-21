#include "UserFunction.h"


#include <utility>

#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UserFunction::UserFunction( const SourceLocation& source_location, bool exported, bool expression,
                            std::string name, std::unique_ptr<FunctionParameterList> parameter_list,
                            std::unique_ptr<FunctionBody> body,
                            const SourceLocation& endfunction_location )
    : Function( source_location, std::move( name ), std::move( parameter_list ),
                std::move( body ) ),
      exported( exported ),
      expression( expression ),
      endfunction_location( endfunction_location )
{
}

void UserFunction::accept( NodeVisitor& visitor )
{
  visitor.visit_user_function( *this );
}

void UserFunction::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "user-function({})", name );
}

FunctionBody& UserFunction::body()
{
  return child<FunctionBody>( 1 );
}

}  // namespace Pol::Bscript::Compiler
