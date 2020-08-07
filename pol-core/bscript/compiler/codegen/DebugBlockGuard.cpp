#include "DebugBlockGuard.h"

#include "InstructionEmitter.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
DebugBlockGuard::DebugBlockGuard( InstructionEmitter& emitter,
                                  const std::vector<std::shared_ptr<Variable>>& debug_variables )
    : emitter( emitter ), previous_block_id( emitter.enter_debug_block( debug_variables ) )
{
}
DebugBlockGuard::~DebugBlockGuard()
{
  emitter.set_debug_block( previous_block_id );
}

}  // namespace Pol::Bscript::Compiler
