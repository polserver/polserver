#ifndef POLSERVER_USERFUNCTION_H
#define POLSERVER_USERFUNCTION_H

#include "compiler/ast/Function.h"
#include "compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class FunctionParameterList;
class FunctionBody;
class Variable;

class UserFunction : public Function
{
public:
  UserFunction( const SourceLocation&, bool exported, std::string name,
                std::unique_ptr<FunctionParameterList>, std::unique_ptr<FunctionBody>,
                const SourceLocation& endfunction_location );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const bool exported;
  const SourceLocation endfunction_location;

  LocalVariableScopeInfo local_variable_scope_info;

  FunctionBody& body();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTION_H
