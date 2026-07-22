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
    if ( loc.id == RSV_JMPIFFALSE )
    {
      if ( jump.type == TYP_LOGICAL_JUMP_FALSE )
        jump.offset = loc.offset;
      else
        jump.offset++;
    }
    else if ( loc.id == RSV_JMPIFTRUE )
    {
      if ( jump.type != TYP_LOGICAL_JUMP_FALSE )
        jump.offset = loc.offset;
      else
        jump.offset++;
    }
    else if ( loc.id == INS_LOGICAL_JUMP )
    {
      if ( loc.type == jump.type )
        jump.offset = loc.offset;
      else
        // opposite-type jump falls through and pops this value; skipping it would
        // leave a stray value on the stack, so keep targeting it
        return;
    }
    else if ( loc.id == INS_LOGICAL_CONVERT )
    {
      jump.offset++;
    }
    else
    {
      return;
    }
    return combine( jump, combine );
  };
  for ( auto& c : code )
    if ( c.id == INS_LOGICAL_JUMP )
      combine( c, combine );
}
}  // namespace Pol::Bscript::Compiler
