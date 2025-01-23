/** @file
 *
 * @par History
 */

#include "eprog.h"

#include <cstdio>

#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "escriptv.h"
#include "fmodule.h"

namespace Pol
{
namespace Bscript
{
EScriptProgram::EScriptProgram()
    : ref_counted(),
      nglobals( 0 ),
      expectedArgs( 0 ),
      haveProgram( false ),
      name( "" ),
      modules(),
      tokens(),
      symbols(),
      exported_functions(),
      version( 0 ),
      invocations( 0 ),
      instr_cycles( 0 ),
      pkg( nullptr ),
      instr(),
      debug_loaded( false ),
      savecurblock( 0 ),
      curblock( 0 ),
      curfile( 0 ),
      curline( 0 ),
      statementbegin( false ),
      globalvarnames(),
      blocks(),
      dbg_functions(),
      dbg_filenames(),
      dbg_filenum(),
      dbg_linenum(),
      dbg_ins_blocks(),
      dbg_ins_statementbegin()
{
  ++escript_program_count;

  // compiler only:
  EPDbgBlock block;
  block.parentblockidx = 0;
  block.parentvariables = 0;
  blocks.push_back( block );
}

EScriptProgram::~EScriptProgram()
{
  Clib::delete_all( modules );
  --escript_program_count;
}

}  // namespace Bscript
}  // namespace Pol
