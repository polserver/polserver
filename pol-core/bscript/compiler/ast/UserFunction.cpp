#include "UserFunction.h"

#include <format/format.h>
#include <utility>

#include "FunctionBody.h"
#include "FunctionParameterList.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
UserFunction::UserFunction( const SourceLocation& source_location, bool exported, std::string name,
                            std::unique_ptr<FunctionParameterList> parameter_list,
                            std::unique_ptr<FunctionBody> body,
                            const SourceLocation& endfunction_location )
  : Function( source_location, std::move( name ), std::move( parameter_list ),
              std::move( body ) ),
    exported( exported ),
    endfunction_location( endfunction_location )
{
}

void UserFunction::accept( NodeVisitor& visitor )
{
  visitor.visit_user_function( *this );
}

void UserFunction::describe_to( fmt::Writer& w ) const
{
  w << "user-function(" << name << ")";
}

FunctionBody& UserFunction::body()
{
  return child<FunctionBody>( 1 );
}

}  // namespace Pol::Bscript::Compiler