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
    if ( loc.id == RSV_JMPIFFALSE || loc.id == RSV_JMPIFTRUE )
    {
      // A conditional jump ALWAYS pops its condition (Executor::ins_jmpiffalse /
      // ins_jmpiftrue), but a firing INS_LOGICAL_JUMP KEEPS its value on the stack.
      // Threading the logical jump onto/past the conditional jump would skip that pop
      // and strand a value, corrupting a following stack-addressed op (e.g. the `:=` of
      // `var x := ( a && b ) ? ... ` or the return value read out of a `do..dowhile`
      // predicate). Keep targeting the conditional jump so its pop still runs.
      return;
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
