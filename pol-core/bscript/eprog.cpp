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
    : Program(),
      nglobals( 0 ),
      expectedArgs( 0 ),
      haveProgram( false ),
      name( "" ),
      modules(),
      tokens(),
      symbols(),
      program_PC( UINT_MAX ),
      program_decl( "" ),
      sourcelines(),
      fileline(),
      function_decls(),
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

//#define _DBG_TRYING_TO_FIND_WIN32_SHUTDOWN_ASSERTION
EScriptProgram::~EScriptProgram()
{
#if defined( _DEBUG ) && defined( _DBG_TRYING_TO_FIND_WIN32_SHUTDOWN_ASSERTION )
  cout << "ESP~D: " << name << endl;
#endif

  Clib::delete_all( modules );
  --escript_program_count;
}

Program* EScriptProgram::create()
{
  return new EScriptProgram();
}

Program::Program() : ref_counted() {

}

void EScriptProgram::clear_modules()
{
  Clib::delete_all( modules );
}

bool EScriptProgram::hasProgram() const
{
  return haveProgram;
}

Program::ProgramType EScriptProgram::type() const
{
  return Program::ProgramType::ESCRIPT;
}

void EScriptProgram::package(const Plib::Package* pkg_) {
  pkg = pkg_;
}

std::string EScriptProgram::scriptname() const {
  return name;
}

}  // namespace Bscript
}  // namespace Pol
