#ifndef POLSERVER_DEBUGBLOCKGUARD_H
#define POLSERVER_DEBUGBLOCKGUARD_H

#include <memory>
#include <vector>

namespace Pol::Bscript::Compiler
{
class InstructionEmitter;
class LocalVariableScopeInfo;
class Variable;

class DebugBlockGuard
{
public:
  DebugBlockGuard( InstructionEmitter&,
                   LocalVariableScopeInfo& );
  ~DebugBlockGuard();

private:
  InstructionEmitter& emitter;
  const unsigned previous_block_id;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGBLOCKGUARD_H
