/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _WIN32
#	include <windows.h>
#endif

#include <string.h>

#ifdef WIN32
#	include <process.h>
#endif

#include <stdio.h>
#include <time.h>

#include <iostream>
#include <iomanip>

#include "../clib/clib.h"
#include "../clib/progver.h"
#include "../clib/rstack.h"

#include "../bscript/config.h"
#include "../bscript/eprog.h"
#include "../bscript/escriptv.h"
#include "../bscript/filefmt.h"
#include "../bscript/tokens.h"
#include "../bscript/symcont.h"
#include "../bscript/token.h"
#include "../bscript/execmodl.h"
#include "../bscript/executor.h"

#include "../pol/sqlscrobj.h"
#include "../clib/cmdargs.h"

#include "../pol/module/basicmod.h"
#include "../pol/module/basiciomod.h"
#include "../pol/module/mathmod.h"
#include "../pol/module/sqlmod.h"

#if REFPTR_DEBUG
	unsigned int ref_counted::_ctor_calls;
#endif

int quiet = 0;
int debug = 0;
bool profile = false;
void usage(void)
{
    cerr << "  Usage:" << endl;
    cerr << "    " << progverstr << " [options] filespec [filespec ...]" << endl;
    cerr << "        Options:" << endl;
    cerr << "            -q    Quiet" << endl;
    cerr << "            -d    Debug output" << endl;
    cerr << "            -p    Profile" << endl;
    exit(1);
}

void DumpCaseJmp( ostream& os, const Token& token, EScriptProgram* prog )
{
    const unsigned char* dataptr = token.dataptr;
    for (;;)
    {
        unsigned short offset = * (const unsigned short*) dataptr;
        dataptr += 2;
        unsigned char type = *dataptr;
        dataptr += 1;
        if (type == CASE_TYPE_LONG)
        {
            unsigned int lval = * (const unsigned int*) dataptr;
            dataptr += 4;
            os << "\t" << lval << ": @" << offset << endl;
        }
        else if (type == CASE_TYPE_DEFAULT)
        {
            os << "\tdefault: @" << offset << endl;
            break;
        }
        else
        { // type is the length of the string, otherwise
            os << "\t\"" << string( (const char*)dataptr, type ) << "\": @" << offset << endl;
            dataptr += type;
        }
    }
}

BObjectImp::BObjectType ot = BObjectImp::OTLong;
BApplicObjType aot;
int x;

void foo( BApplicObjType* laot, BObjectImp::BObjectType lot )
{
    if (laot == &aot)
    {
        x = 5;
    }

    if (lot == BObjectImp::OTLong)
    {
        x = 6;
    }
}

void DumpScript( const char *path )
{
    string fname( path );
    if (fname.size() >= 4)
        fname.replace( fname.size()-4, 4, ".ecl" );

    Executor E(cerr);
    E.setViewMode(true);
    E.addModule( new BasicExecutorModule(E) );
    E.addModule( new BasicIoExecutorModule(E, cout) );
    E.addModule( new MathExecutorModule(E) );
    E.addModule( new SQLExecutorModule(E));

    ref_ptr<EScriptProgram> program( new EScriptProgram );
    program->read( fname.c_str() );
    E.setProgram( program.get() );

    program->dump( cout );
    return;
	
	Token token;
	unsigned PC;
	for( PC = 0; PC < E.nLines; PC++ )
	{
	    if (E.getToken(token, PC)) 
		{
		    return;
		}
		else
		{
			cout << PC << ": " << token;
            if (debug) cout << " (" << token.id << "," << token.type << ")";
            cout << endl;
            if (token.id == INS_CASEJMP)
            {
                DumpCaseJmp( cout, token, program.get() );
            }
		}
	}
}


int exec_script(const char *path)
{
    string fname( path );
    // TODO: autoconvert to .ecl ?

    Executor E(cerr);
    E.addModule( new BasicExecutorModule(E) );
    E.addModule( new BasicIoExecutorModule(E, cout) );
    E.addModule( new MathExecutorModule(E) );
    E.addModule( new SQLExecutorModule(E) );

    ref_ptr<EScriptProgram> program( new EScriptProgram );
    if (program->read( fname.c_str() ))
    {
        cerr << "Error reading " << fname << endl;
        return 1;
    }
    E.setProgram( program.get() );

    E.setDebugLevel(debug ? Executor::INSTRUCTIONS : Executor::NONE);
    clock_t start = clock();
#ifdef _WIN32
    FILETIME dummy;
    FILETIME kernelStart, userStart;
    GetThreadTimes( GetCurrentThread(), &dummy, &dummy, &kernelStart, &userStart );
#endif
    bool exres = E.exec();
#ifdef _WIN32
    FILETIME kernelEnd, userEnd;
    GetThreadTimes( GetCurrentThread(), &dummy, &dummy, &kernelEnd, &userEnd );
#endif
    clock_t clocks = clock()-start;
    double seconds = static_cast<double>(clocks)/CLOCKS_PER_SEC;

    if (profile)
    {
        cout << "Profiling information: " << endl;
        cout << "\tEObjectImp constructions: " << eobject_imp_constructions << endl;
        if (eobject_imp_count)
            cout << "\tRemaining BObjectImps: " << eobject_imp_count << endl;
        cout << "\tInstruction cycles: " << escript_instr_cycles << endl;
        cout << "\tInnerExec calls: " << escript_execinstr_calls << endl;
        cout << "\tClocks: " << clocks << " (" << seconds << " seconds)" << endl;
#ifdef _WIN32
        cout << "\tKernel Time: " << (*(__int64*)&kernelEnd) - (*(__int64*)&kernelStart) << endl;
        cout << "\tUser Time:   " << (*(__int64*)&userEnd) - (*(__int64*)&userStart) << endl;
#endif
        cout << "\tSpace used: " << E.sizeEstimate() << endl;
        cout << endl;
        cout.precision(0);
        cout << "\tCycles Per Second: " << std::fixed << std::noshowpoint << setw(14) << escript_instr_cycles / seconds << endl;
        cout << "\tCycles Per Minute: " << std::fixed << std::noshowpoint << setw(14) << 60.0 * escript_instr_cycles / seconds << endl;
        cout << "\tCycles Per Hour:   " << std::fixed << std::noshowpoint << setw(14) << 3600.0 * escript_instr_cycles / seconds << endl;

#ifdef ESCRIPT_PROFILE
		for (escript_profile_map::iterator itr=EscriptProfileMap.begin();itr!=EscriptProfileMap.end();++itr)
		{
			cout << itr->first << "," << itr->second.count << "," << itr->second.min << "," << itr->second.max << "," << itr->second.sum << "," 
				<< (itr->second.sum / itr->second.count) << endl;
		}
#endif
    }
    return exres ? 0 : 1;
}

int run(int argc, char **argv)
{
    for(int i=1;i<argc;i++) 
    {
        switch(argv[i][0]) 
        {
            case '/': case '-':
                switch(argv[i][1]) 
                {
                    case 'a': case 'A':
                    case 'd': case 'D':
                    case 'v': case 'V':
                    case 'q': case 'Q':
                    case 'p': case 'P':
                        break;
                    default:
                        cerr << "Unknown option: " << argv[i] << endl;
                        usage();
                        return 1;
                }
                break;
            default:
                return exec_script( argv[i] );
        }
    }
    return 0;
}

int xmain(int argc, char *argv[])
{
    StoreCmdArgs( argc, argv );
    
    strcpy( progverstr, "RUNECL" );
    progver = 1;

	escript_config.max_call_depth = 100;
    quiet = FindArg("q") ? 1 : 0;
    debug = FindArg("d") ? 1 : 0;
    profile = FindArg( "p" ) ? 1 : 0;
    passert_disabled = FindArg( "a" ) ? false : true;

    if (!quiet)
    {
		// vX.YY
		double vernum = (double)progver + (double)(ESCRIPT_FILE_VER_CURRENT / 100.0f);
        cerr << "EScript Executor v" << vernum << endl;
        cerr << "Copyright (C) 1993-2011 Eric N. Swanson" << endl;
        cerr << endl;
    }

    if (argc==1) 
    {
        usage();
        return 1;
    }

    const char *todump = FindArg( "v" );
    if (todump)
    {
        DumpScript( todump );
        return 0;
    }

    return run(argc, argv);
}
