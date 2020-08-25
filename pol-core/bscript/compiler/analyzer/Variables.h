#ifndef POLSERVER_VARIABLES_H
#define POLSERVER_VARIABLES_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "clib/maputil.h"
#include "compiler/model/VariableScope.h"

namespace Pol::Bscript::Compiler
{
class Report;
class SourceLocation;
class Variable;

class Variables
{
public:
  Variables( VariableScope, Report& );

  std::shared_ptr<Variable> create( const std::string& name, unsigned block_depth,
                                    bool warn_if_used, bool warn_if_unused, const SourceLocation& );

  std::shared_ptr<Variable> find( const std::string& name ) const;

  const std::vector<std::string>& get_names() const;
  unsigned count() const;

private:
  const VariableScope scope;
  Report& report;

  typedef std::map<std::string, std::shared_ptr<Variable>, Clib::ci_cmp_pred> VariableMap;

  VariableMap variables_by_name;
  std::vector<std::string> names_by_index;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SCRIPTVARIABLES_H
