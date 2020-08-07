#ifndef POLSERVER_FUNCTIONLINK_H
#define POLSERVER_FUNCTIONLINK_H

#include "compiler/file/SourceLocation.h"

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

  Function* function() const;
  ModuleFunctionDeclaration* module_function_declaration() const;
  UserFunction* user_function() const;

  void link_to( Function* );

  const SourceLocation source_location;

private:
  Function* linked_function;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONLINK_H
