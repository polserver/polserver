#include "bscript/compiler/optimizer/CodeSectionOptimizer.h"
#include "bscript/StoredToken.h"
#include "bscript/compilercfg.h"
#include "bscript/tokens.h"

namespace Pol::Bscript::Compiler
{
void CodeSectionOptimizer::optimize( CodeSection& code ) const
{
  if ( compilercfg.ShortCircuitEvaluation )
    short_circuit_jumps( code );
}

void CodeSectionOptimizer::short_circuit_jumps( CodeSection& code ) const
{
  // recursivly check if a logical jump would jump to another jump and update the final jump
  // location
  auto combine = [&]( StoredToken& jump, auto&& combine )
  {
    const auto& loc = code[jump.offset];
    // logical jumps of different type pop value from stack
    // same is true for "normal" jumps
    // thus they cannot be optimized
    if ( loc.id == INS_LOGICAL_JUMP && loc.type == jump.type )
      jump.offset = loc.offset;
    else if ( loc.id == INS_LOGICAL_CONVERT )
      jump.offset++;
    else
      return;
    return combine( jump, combine );
  };
  for ( auto& c : code )
    if ( c.id == INS_LOGICAL_JUMP )
      combine( c, combine );
}
}  // namespace Pol::Bscript::Compiler
