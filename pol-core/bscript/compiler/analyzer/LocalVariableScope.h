#ifndef POLSERVER_LOCALVARIABLESCOPE_H
#define POLSERVER_LOCALVARIABLESCOPE_H

#include <memory>
#include <string>
#include <vector>

#include "compiler/model/WarnOn.h"

namespace Pol::Bscript::Compiler
{
class Report;
class LocalVariableScopes;
class SourceLocation;
class Variable;

class LocalVariableScope
{
public:
  explicit LocalVariableScope( LocalVariableScopes&,
                               std::vector<std::shared_ptr<Variable>>& debug_variables );
  ~LocalVariableScope();

  std::shared_ptr<Variable> create( const std::string& name, WarnOn, const SourceLocation& );

  [[nodiscard]] unsigned get_block_locals() const;
private:
  LocalVariableScopes& scopes;
  Report& report;
  const unsigned block_depth;
  const unsigned prev_locals;
  std::vector<std::shared_ptr<Variable>> shadowing;
  std::vector<std::shared_ptr<Variable>>& debug_variables;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LOCALVARIABLESCOPE_H
