#ifndef POLSERVER_VARIABLE_H
#define POLSERVER_VARIABLE_H

#include <string>

#include "compiler/model/VariableScope.h"
#include "compiler/model/WarnOn.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;

class Variable
{
public:
  Variable( VariableScope, std::string name, unsigned block_depth, size_t index, WarnOn,
            const SourceLocation& source_location );

  void mark_used();
  [[nodiscard]] bool was_used() const;

  const VariableScope scope;
  const std::string name;
  const unsigned block_depth;
  const size_t index;
  const WarnOn warn_on;
  const SourceLocation& source_location;

private:
  bool used;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SCRIPTVARIABLE_H
