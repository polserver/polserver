#include "InstructionGenerator.h"

#include "InstructionEmitter.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator( InstructionEmitter& emitter )
  : emitter( emitter ),
    emit( emitter )
{
}

}  // namespace Pol::Bscript::Compiler
