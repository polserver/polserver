#ifndef POLSERVER_LOCALVARIABLESCOPE_H
#define POLSERVER_LOCALVARIABLESCOPE_H

#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class Report;
class Scopes;
class SourceLocation;
class Variable;

class LocalVariableScope
{
public:
  explicit LocalVariableScope( Scopes&,
                               std::vector<std::shared_ptr<Variable>>& debug_variables );
  ~LocalVariableScope();

  std::shared_ptr<Variable> create( const std::string& name, bool warn_if_used, bool warn_if_unused,
                                    const SourceLocation& );

  unsigned get_block_locals() const;
private:
  Scopes& scopes;
  Report& report;
  const unsigned block_depth;
  const unsigned prev_locals;
  std::vector<std::shared_ptr<Variable>> shadowing;
  std::vector<std::shared_ptr<Variable>>* const debug_variables;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LOCALVARIABLESCOPE_H
