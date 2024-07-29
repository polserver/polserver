#ifndef POLSERVER_VARIABLES_H
#define POLSERVER_VARIABLES_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/model/SimpleTypes.h"
#include "bscript/compiler/model/VariableScope.h"
#include "bscript/compiler/model/WarnOn.h"
#include "clib/maputil.h"

namespace Pol::Bscript::Compiler
{
class Report;
class SourceLocation;
class Variable;
class FunctionVariableScope;

class Variables
{
public:
  Variables( VariableScope, Report& );

  std::shared_ptr<Variable> create( const std::string& name, BlockDepth, WarnOn,
                                    const SourceLocation& );

  std::shared_ptr<Variable> capture( std::shared_ptr<Variable>& );

  // Looks in parent and all ancestor function variable scopes
  [[nodiscard]] std::shared_ptr<Variable> find_in_ancestors( const std::string& name ) const;

  // Only looks in current function variable scope
  [[nodiscard]] std::shared_ptr<Variable> find( const std::string& name ) const;

  void restore_shadowed( std::shared_ptr<Variable> );

  void remove_all_but( unsigned count );

  [[nodiscard]] const std::vector<std::string>& get_names() const;
  [[nodiscard]] unsigned count() const;

private:
  const VariableScope scope;
  Report& report;

  // Encapsulates the current variable stack across function expresions
  struct VariablesInfo
  {
    typedef std::map<std::string, std::shared_ptr<Variable>, Clib::ci_cmp_pred> VariableMap;

    VariableMap variables_by_name;
    std::vector<std::string> names_by_index;
  };

  std::vector<VariablesInfo> variable_info_stack;
  VariablesInfo& current();

  // Used to reset the variable indexs for new function expressions
  friend class FunctionVariableScope;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SCRIPTVARIABLES_H
