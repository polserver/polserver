#ifndef POLSERVER_FUNCTIONLINK_H
#define POLSERVER_FUNCTIONLINK_H

#include "bscript/compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class Function;
class ModuleFunctionDeclaration;
class ScopeName;
class UserFunction;

class FunctionLink
{
public:
  explicit FunctionLink( const SourceLocation&, std::string calling_scope );
  FunctionLink( const FunctionLink& ) = delete;
  FunctionLink& operator=( const FunctionLink& ) = delete;

  [[nodiscard]] Function* function() const;
  [[nodiscard]] ModuleFunctionDeclaration* module_function_declaration() const;
  [[nodiscard]] UserFunction* user_function() const;

  void link_to( Function* );

  const SourceLocation source_location;

  // eg. `Animal` when calling any function inside the `Animal` class
  const std::string calling_scope;

private:
  Function* linked_function;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONLINK_H
