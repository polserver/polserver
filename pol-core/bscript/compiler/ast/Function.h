#ifndef POLSERVER_FUNCTION_H
#define POLSERVER_FUNCTION_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class FunctionParameterDeclaration;
class FunctionParameterList;
class FunctionBody;

class Function : public Node
{
public:
  Function( const SourceLocation&, std::string scope, std::string name,
            std::unique_ptr<FunctionParameterList>, std::unique_ptr<FunctionBody> );
  Function( const SourceLocation&, std::string scope, std::string name,
            std::unique_ptr<FunctionParameterList> );

  unsigned parameter_count() const;
  bool is_variadic() const;
  // TODO implement ScopableName and update GeneratedFunctionBuilder
  std::string scoped_name() const;
  std::vector<std::reference_wrapper<FunctionParameterDeclaration>> parameters();

  // TODO rename this to `scope`. Keeping as module_name for now to avoid too
  // many changes.
  const std::string scope;
  const std::string name;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTION_H
