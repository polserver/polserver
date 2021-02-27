#ifndef POLSERVER_FUNCTIONLINK_H
#define POLSERVER_FUNCTIONLINK_H

#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class Function;
class ModuleFunctionDeclaration;
class UserFunction;

class FunctionLink
{
public:
  explicit FunctionLink( const SourceLocation& );
  FunctionLink( const FunctionLink& ) = delete;
  FunctionLink& operator=( const FunctionLink& ) = delete;

  [[nodiscard]] Function* function() const;
  [[nodiscard]] ModuleFunctionDeclaration* module_function_declaration() const;
  [[nodiscard]] UserFunction* user_function() const;

  void link_to( Function* );

  const SourceLocation source_location;

private:
  Function* linked_function;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONLINK_H
