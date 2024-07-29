#ifndef POLSERVER_USERFUNCTION_H
#define POLSERVER_USERFUNCTION_H

#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class FunctionParameterList;
class FunctionBody;
class Variable;

class UserFunction : public Function
{
public:
  UserFunction( const SourceLocation&, bool exported, bool expression, std::string name,
                std::unique_ptr<FunctionParameterList>, std::unique_ptr<FunctionBody>,
                const SourceLocation& endfunction_location );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool exported;
  const bool expression;
  const SourceLocation endfunction_location;

  LocalVariableScopeInfo local_variable_scope_info;
  LocalVariableScopeInfo capture_variable_scope_info;

  unsigned capture_count() const;

  FunctionBody& body();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTION_H
