#include "InstructionEmitter.h"

#include "StoredToken.h"
#include "compiler/representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{
InstructionEmitter::InstructionEmitter( CodeSection& code, DataSection& data )
  : code_emitter( code ),
    data_emitter( data )
{
  initialize_data();
}

void InstructionEmitter::initialize_data()
{
  std::byte nul{};
  data_emitter.store( &nul, sizeof nul );
}

}  // namespace Pol::Bscript::Compiler
