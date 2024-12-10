#ifndef POLSERVER_USERFUNCTION_H
#define POLSERVER_USERFUNCTION_H

#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"
#include "bscript/compiler/model/UserFunctionType.h"

namespace Pol::Bscript::Compiler
{
class ClassLink;
class FunctionParameterList;
class FunctionBody;
class TypeNode;
class TypeParameterList;
class Variable;

class UserFunction : public Function
{
public:
  UserFunction( const SourceLocation&, bool exported, bool expression, UserFunctionType type,
                std::string scope, std::string name, std::unique_ptr<TypeParameterList>,
                std::unique_ptr<FunctionParameterList>, std::unique_ptr<FunctionBody>,
                std::unique_ptr<TypeNode>, const SourceLocation& endfunction_location,
                std::shared_ptr<ClassLink> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool exported;
  const bool expression;
  const UserFunctionType type;
  const SourceLocation endfunction_location;
  std::shared_ptr<ClassLink> class_link;

  LocalVariableScopeInfo local_variable_scope_info;
  LocalVariableScopeInfo capture_variable_scope_info;

  unsigned capture_count() const;

  FunctionBody& body();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTION_H
