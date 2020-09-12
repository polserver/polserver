#include "DebugBlockGuard.h"

#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/model/LocalVariableScopeInfo.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
DebugBlockGuard::DebugBlockGuard( InstructionEmitter& emitter,
                                  LocalVariableScopeInfo& local_variable_scope_info )
    : emitter( emitter ),
      previous_block_id( emitter.enter_debug_block( local_variable_scope_info ) )
{
}
DebugBlockGuard::~DebugBlockGuard()
{
  emitter.set_debug_block( previous_block_id );
}

}  // namespace Pol::Bscript::Compiler
