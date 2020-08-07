#ifndef POLSERVER_FUNCTION_H
#define POLSERVER_FUNCTION_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class FunctionParameterDeclaration;
class FunctionParameterList;
class FunctionBody;

class Function : public Node
{
public:
  Function( const SourceLocation&, std::string name, std::unique_ptr<FunctionParameterList>,
            std::unique_ptr<FunctionBody> );
  Function( const SourceLocation&, std::string name, std::unique_ptr<FunctionParameterList> );

  unsigned parameter_count() const;
  std::vector<std::reference_wrapper<FunctionParameterDeclaration>> parameters();

  const std::string name;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTION_H
