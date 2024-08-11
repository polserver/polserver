#include "DefaultConstructorFunction.h"

#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{

DefaultConstructorFunction::DefaultConstructorFunction( const SourceLocation& loc,
                                                        const std::string& class_name )
    : UserFunction(
          loc, false, false, UserFunctionType::Constructor, class_name /* scope */,
          class_name /* function name */,
          std::make_unique<FunctionParameterList>(
              loc, std::vector<std::unique_ptr<FunctionParameterDeclaration>>() ),
          std::make_unique<FunctionBody>( loc, std::vector<std::unique_ptr<Statement>>() ), loc )
{
  child<FunctionParameterList>( 0 ).children.push_back(
      std::make_unique<FunctionParameterDeclaration>( loc, "this", true /*byref*/, false /*unused*/,
                                                      false /*rest*/ ) );
}
void DefaultConstructorFunction::accept( NodeVisitor& visitor )
{
  visitor.visit_default_constructor_function( *this );
}

void DefaultConstructorFunction::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "default-ctor({})", name );
}

}  // namespace Pol::Bscript::Compiler
