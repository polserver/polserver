#include "UserFunction.h"


#include <utility>

#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/ClassLink.h"

namespace Pol::Bscript::Compiler
{
UserFunction::UserFunction( const SourceLocation& source_location, bool exported, bool expression,
                            UserFunctionType type, std::string scope, std::string name,
                            std::unique_ptr<FunctionParameterList> parameter_list,
                            std::unique_ptr<FunctionBody> body,
                            const SourceLocation& endfunction_location,
                            std::shared_ptr<ClassLink> class_link )
    : Function( source_location, std::move( scope ), std::move( name ), std::move( parameter_list ),
                std::move( body ) ),
      exported( exported ),
      expression( expression ),
      type( type ),
      endfunction_location( endfunction_location ),
      class_link( std::move( class_link ) )
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

unsigned UserFunction::capture_count() const
{
  return static_cast<unsigned>( capture_variable_scope_info.variables.size() );
}

FunctionBody& UserFunction::body()
{
  return child<FunctionBody>( 1 );
}

}  // namespace Pol::Bscript::Compiler
