#ifndef POLSERVER_DEBUGBLOCKGUARD_H
#define POLSERVER_DEBUGBLOCKGUARD_H

#include <memory>
#include <vector>

namespace Pol::Bscript::Compiler
{
class InstructionEmitter;
class Variable;

class DebugBlockGuard
{
public:
  DebugBlockGuard( InstructionEmitter&,
                   const std::vector<std::shared_ptr<Variable>>& debug_variables );
  ~DebugBlockGuard();

private:
  InstructionEmitter& emitter;
  const unsigned previous_block_id;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGBLOCKGUARD_H
