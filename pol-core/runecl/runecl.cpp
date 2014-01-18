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
#include "../clib/logfacility.h"

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
#include "../clib/cfgelem.h"

#include "../pol/module/basicmod.h"
#include "../pol/module/basiciomod.h"
#include "../pol/module/mathmod.h"
#include "../pol/module/sqlmod.h"
#include "../pol/module/utilmod.h"
#include "../pol/module/filemod.h"
#include "../pol/module/cfgmod.h"
#include "../pol/module/datastore.h"

#include "../pol/polcfg.h"

#include "../clib/timer.h"

namespace Pol {
  namespace Core {
    PolConfig config; // needed but not really used def
  }
  namespace Bscript {
    void display_bobjectimp_instances();
  }
  namespace Runecl {
    using namespace Bscript;
    using namespace Module;
#if REFPTR_DEBUG
    unsigned int ref_counted::_ctor_calls;
#endif

    int quiet = 0;
    int debug = 0;
    bool profile = false;
    void usage( void )
    {
      ERROR_PRINT << "  Usage:\n"
        << "    " << progverstr << " [options] filespec [filespec ...]\n"
        << "        Options:\n"
        << "            -q    Quiet\n"
        << "            -d    Debug output\n"
        << "            -p    Profile\n";
    }

    void DumpCaseJmp( ostream& os, const Token& token, EScriptProgram* prog )
    {
      const unsigned char* dataptr = token.dataptr;
      for ( ;; )
      {
        unsigned short offset = *(const unsigned short*)dataptr;
        dataptr += 2;
        unsigned char type = *dataptr;
        dataptr += 1;
        if ( type == CASE_TYPE_LONG )
        {
          unsigned int lval = *(const unsigned int*)dataptr;
          dataptr += 4;
          os << "\t" << lval << ": @" << offset << endl;
        }
        else if ( type == CASE_TYPE_DEFAULT )
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

    Bscript::BObjectImp::BObjectType ot = Bscript::BObjectImp::OTLong;
    BApplicObjType aot;
    int x;

    void foo( BApplicObjType* laot, Bscript::BObjectImp::BObjectType lot )
    {
      if ( laot == &aot )
      {
        x = 5;
      }

      if ( lot == Bscript::BObjectImp::OTLong )
      {
        x = 6;
      }
    }

    void DumpScript( const char *path )
    {
      string fname( path );
      if ( fname.size() >= 4 )
        fname.replace( fname.size() - 4, 4, ".ecl" );

      Executor E;
      E.setViewMode( true );
      E.addModule( new BasicExecutorModule( E ) );
      E.addModule( new BasicIoExecutorModule( E ) );
      E.addModule( new MathExecutorModule( E ) );
      E.addModule( new SQLExecutorModule( E ) );
      E.addModule( new UtilExecutorModule( E ) );
      E.addModule( new FileAccessExecutorModule( E ) );
      E.addModule( new ConfigFileExecutorModule( E ) );
      E.addModule( new DataFileExecutorModule( E ) );

      ref_ptr<EScriptProgram> program( new EScriptProgram );
      program->read( fname.c_str() );
      E.setProgram( program.get() );

      std::ostringstream os;
      program->dump( os );
      INFO_PRINT << os.str();

      /* return;

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
          */
    }

    
    int exec_script( const char *path )
    {
      string fname( path );
      // TODO: autoconvert to .ecl ?

      Executor E;
      E.addModule( new BasicExecutorModule( E ) );
      E.addModule( new BasicIoExecutorModule( E ) );
      E.addModule( new MathExecutorModule( E ) );
      E.addModule( new SQLExecutorModule( E ) );
      E.addModule( new UtilExecutorModule( E ) );
      E.addModule( new FileAccessExecutorModule( E ) );
      E.addModule( new ConfigFileExecutorModule( E ) );
      E.addModule( new DataFileExecutorModule( E ) );

      ref_ptr<EScriptProgram> program( new EScriptProgram );
      if ( program->read( fname.c_str() ) )
      {
        ERROR_PRINT << "Error reading " << fname << "\n";
        return 1;
      }
      E.setProgram( program.get() );

      E.setDebugLevel( debug ? Executor::INSTRUCTIONS : Executor::NONE );
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
      clock_t clocks = clock() - start;
      double seconds = static_cast<double>( clocks ) / CLOCKS_PER_SEC;

      if ( profile )
      {
        fmt::Writer tmp;
        tmp << "Profiling information: \n"
          << "\tEObjectImp constructions: " << eobject_imp_constructions << "\n";
        if ( eobject_imp_count )
          tmp << "\tRemaining BObjectImps: " << eobject_imp_count << "\n";
        tmp << "\tInstruction cycles: " << escript_instr_cycles << "\n"
          << "\tInnerExec calls: " << escript_execinstr_calls << "\n"
          << "\tClocks: " << clocks << " (" << seconds << " seconds)" << "\n"
#ifdef _WIN32
          << "\tKernel Time: " << ( *(__int64*)&kernelEnd ) - ( *(__int64*)&kernelStart ) << "\n"
          << "\tUser Time:   " << ( *(__int64*)&userEnd ) - ( *(__int64*)&userStart ) << "\n"
#endif
          << "\tSpace used: " << E.sizeEstimate() << "\n\n"

          << "\tCycles Per Second: " << escript_instr_cycles / seconds << "\n"
          << "\tCycles Per Minute: " << 60.0 * escript_instr_cycles / seconds << "\n"
          << "\tCycles Per Hour:   " << 3600.0 * escript_instr_cycles / seconds << "\n";
#if BOBJECTIMP_DEBUG
        display_bobjectimp_instances();
#endif
#ifdef ESCRIPT_PROFILE
        tmp << "FuncName,Count,Min,Max,Sum,Avarage\n";
        for (escript_profile_map::iterator itr=EscriptProfileMap.begin();itr!=EscriptProfileMap.end();++itr)
        {
          tmp << itr->first << "," << itr->second.count << "," << itr->second.min << "," << itr->second.max << "," << itr->second.sum << "," 
            << (itr->second.sum / itr->second.count) << "\n";
        }
#endif
        INFO_PRINT << tmp.c_str();
      }
      return exres ? 0 : 1;
    }

    int run( int argc, char **argv )
    {
      for ( int i = 1; i < argc; i++ )
      {
        switch ( argv[i][0] )
        {
          case '/': case '-':
            switch ( argv[i][1] )
            {
              case 'a': case 'A':
              case 'd': case 'D':
              case 'v': case 'V':
              case 'q': case 'Q':
              case 'p': case 'P':
                break;
              default:
                ERROR_PRINT << "Unknown option: " << argv[i] << "\n";
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
  }
  int xmain( int argc, char *argv[] )
  {
    Clib::StoreCmdArgs( argc, argv );

    strcpy( progverstr, "RUNECL" );
    progver = 1;

    Runecl::escript_config.max_call_depth = 100;
    Runecl::quiet = Clib::FindArg( "q" ) ? 1 : 0;
    Runecl::debug = Clib::FindArg( "d" ) ? 1 : 0;
    Runecl::profile = Clib::FindArg( "p" ) ? 1 : 0;
    Clib::passert_disabled = Clib::FindArg( "a" ) ? false : true;

    if ( !Runecl::quiet )
    {
      // vX.YY
      double vernum = (double)progver + (double)( ESCRIPT_FILE_VER_CURRENT / 100.0f );
      ERROR_PRINT << "EScript Executor v" << vernum << "\n"
      << "Copyright (C) 1993-2014 Eric N. Swanson\n\n";
    }

    if ( argc == 1 )
    {
      Runecl::usage( );
      return 1;
    }

    const char *todump = Clib::FindArg( "v" );
    if ( todump )
    {
      Runecl::DumpScript( todump );
      return 0;
    }

    return Runecl::run( argc, argv );
  }
}