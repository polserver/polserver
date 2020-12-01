#ifndef POLSERVER_LOCALVARIABLESCOPE_H
#define POLSERVER_LOCALVARIABLESCOPE_H

#include <memory>
#include <string>
#include <vector>

#include "compiler/model/SimpleTypes.h"
#include "compiler/model/WarnOn.h"

namespace Pol::Bscript::Compiler
{
class Report;
class LocalVariableScopeInfo;
class LocalVariableScopes;
class SourceLocation;
class Variable;

class LocalVariableScope
{
public:
  explicit LocalVariableScope( LocalVariableScopes&,
                               LocalVariableScopeInfo& );
  ~LocalVariableScope();

  std::shared_ptr<Variable> create( const std::string& name, WarnOn, const SourceLocation& );

private:
  LocalVariableScopes& scopes;
  Report& report;
  const BlockDepth block_depth;
  const unsigned prev_locals;
  std::vector<std::shared_ptr<Variable>> shadowing;
  LocalVariableScopeInfo& local_variable_scope_info;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LOCALVARIABLESCOPE_H
