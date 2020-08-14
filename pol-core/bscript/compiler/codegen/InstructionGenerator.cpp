#include "InstructionGenerator.h"

#include "compiler/codegen/InstructionEmitter.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator( InstructionEmitter& emitter )
  : emitter( emitter ),
    emit( emitter )
{
}

}  // namespace Pol::Bscript::Compiler
