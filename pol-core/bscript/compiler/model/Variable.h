#ifndef POLSERVER_VARIABLE_H
#define POLSERVER_VARIABLE_H

#include <memory>
#include <string>

#include "bscript/compiler/model/SimpleTypes.h"
#include "bscript/compiler/model/VariableScope.h"
#include "bscript/compiler/model/WarnOn.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;

class Variable
{
public:
  Variable( VariableScope, std::string name, BlockDepth, VariableIndex, WarnOn,
            std::shared_ptr<Variable> capturing, const SourceLocation& source_location,
            const SourceLocation& var_decl_location );


  void mark_used();
  [[nodiscard]] bool was_used() const;

  const VariableScope scope;
  const std::string name;
  const BlockDepth block_depth;
  const VariableIndex index;
  const WarnOn warn_on;
  const SourceLocation& source_location;
  std::shared_ptr<Variable> capturing;
  const SourceLocation& var_decl_location;

private:
  bool used;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SCRIPTVARIABLE_H
