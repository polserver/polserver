/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>

#include "../clib/stlutil.h"

#include "filefmt.h"

#include "eprog.h"
#include "escriptv.h"
#include "options.h"
#include "executortype.h"

EScriptProgram::EScriptProgram() :
    ref_counted(),
    name(""),
    nglobals(0),
    modules(),
    tokens(),
    symbols(),
    haveProgram(false),
    expectedArgs(0),
    sourcelines(),
    fileline(),
    program_PC( UINT_MAX ),
    program_decl(""),
    function_decls(),
    version(0),
    instr(),
    invocations(0),
    instr_cycles(0),
    pkg(NULL),
    debug_loaded(false),
    globalvarnames(),
    blocks(),
    dbg_functions(),
    savecurblock(0),
    curblock(0),
    curfile(0),
    curline(0),
    statementbegin(false),
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
#if defined(_DEBUG) && defined(_DBG_TRYING_TO_FIND_WIN32_SHUTDOWN_ASSERTION)
	cout << "ESP~D: " << name << endl;
#endif

    delete_all( modules );
    --escript_program_count;
}

void EScriptProgram::clear_modules()
{
    delete_all( modules );
}

