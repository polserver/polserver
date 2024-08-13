#include "SuperFunction.h"

#include <set>

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/UserFunction.h"

namespace Pol::Bscript::Compiler
{
SuperFunction::SuperFunction( const SourceLocation& loc, ClassDeclaration* cd )
    : UserFunction(
          loc, false, false, UserFunctionType::Super, cd->name /* scope */,
          "super" /* function name */,
          std::make_unique<FunctionParameterList>(
              loc, std::vector<std::unique_ptr<FunctionParameterDeclaration>>() ),
          std::make_unique<FunctionBody>( loc, std::vector<std::unique_ptr<Statement>>() ), loc,
          nullptr ),
      class_declaration( cd )
{
}

void SuperFunction::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "super-function({})", name );
}
}  // namespace Pol::Bscript::Compiler
