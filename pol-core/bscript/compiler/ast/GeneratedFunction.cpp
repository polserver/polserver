#include "GeneratedFunction.h"

#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/types/AnyKeyword.h"
#include "bscript/compiler/ast/types/TypeNode.h"
#include "bscript/compiler/ast/types/TypeParameter.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"
#include "bscript/compiler/model/ClassLink.h"

namespace Pol::Bscript::Compiler
{
GeneratedFunction::GeneratedFunction( const SourceLocation& loc, ClassDeclaration* cd,
                                      UserFunctionType type, const std::string& name )
    : UserFunction(
          loc, false /* exported */, false /* expression */, type, cd->name /* scope */, name,
          std::make_unique<TypeParameterList>( loc, std::vector<std::unique_ptr<TypeParameter>>() ),
          std::make_unique<FunctionParameterList>(
              loc, std::vector<std::unique_ptr<FunctionParameterDeclaration>>() ),
          std::make_unique<FunctionBody>( loc, std::vector<std::unique_ptr<Statement>>() ),
          std::make_unique<AnyKeyword>( loc ), loc, std::make_shared<ClassLink>( loc, cd->name ) )
{
  class_link->link_to( cd );
}

void GeneratedFunction::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "generated-function({})", name );
}

ClassDeclaration* GeneratedFunction::class_declaration() const
{
  return class_link->class_declaration();
}
}  // namespace Pol::Bscript::Compiler
