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

  // Function does not use ScopableName. ScopableName is only needed when we
  // must differentiate between an _empty_ scope and a _global_ scope. This is
  // not the case for Functions, as there is no difference between empty and
  // global.
  std::string scoped_name() const;
  std::vector<std::reference_wrapper<FunctionParameterDeclaration>> parameters();

  const std::string scope;
  const std::string name;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTION_H
