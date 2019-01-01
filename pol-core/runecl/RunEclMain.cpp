
#include "pol_global_config.h"

#include "RunEclMain.h"

#include <ctime>
#include <format/format.h>
#include <iostream>

#include "../bscript/bobject.h"
#include "../bscript/config.h"
#include "../bscript/escriptv.h"
#include "../bscript/execmodl.h"
#include "../bscript/executor.h"
#include "../bscript/filefmt.h"
#include "../bscript/object.h"
#include "../clib/Program/ProgramMain.h"
#include "../clib/boostutils.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"

#include "../pol/globals/ucfg.h"
#include "../pol/module/basiciomod.h"
#include "../pol/module/basicmod.h"
#include "../pol/module/cfgmod.h"
#include "../pol/module/datastore.h"
#include "../pol/module/filemod.h"
#include "../pol/module/mathmod.h"
#include "../pol/module/utilmod.h"
#include "../pol/polcfg.h"

namespace Pol
{
namespace Core
{
// See comment in boost_utils::flyweight_initializers
boost_utils::flyweight_initializers fw_inits;
}  // namespace Core
namespace Module
{
void load_fileaccess_cfg();
}
namespace Clib
{
using namespace std;
using namespace Pol::Bscript;
using namespace Pol::Module;

///////////////////////////////////////////////////////////////////////////////

RunEclMain::RunEclMain() : ProgramMain(), m_quiet( false ), m_debug( false ), m_profile( false ) {}
RunEclMain::~RunEclMain() {}
///////////////////////////////////////////////////////////////////////////////

void RunEclMain::showHelp()
{
  ERROR_PRINT << "Usage:\n"
              << "    \n"
              << "  RUNECL [options] filespec [filespec ...]\n"
              << "        Options:\n"
              << "            -q    Quiet\n"
              << "            -d    Debug output\n"
              << "            -p    Profile\n";
  // TODO: what about "-v" and "-a"?
}

void RunEclMain::dumpScript( std::string fileName )
{
  if ( fileName.size() >= 4 )
    fileName.replace( fileName.size() - 4, 4, ".ecl" );

  Executor exe;
  exe.setViewMode( true );
  exe.addModule( new BasicExecutorModule( exe ) );
  exe.addModule( new BasicIoExecutorModule( exe ) );
  exe.addModule( new MathExecutorModule( exe ) );
  exe.addModule( new UtilExecutorModule( exe ) );
  exe.addModule( new FileAccessExecutorModule( exe ) );
  exe.addModule( new ConfigFileExecutorModule( exe ) );
  exe.addModule( new DataFileExecutorModule( exe ) );

  ref_ptr<EScriptProgram> program( new EScriptProgram );
  program->read( fileName.c_str() );
  exe.setProgram( program.get() );

  std::ostringstream os;
  program->dump( os );
  INFO_PRINT << os.str();
}

int RunEclMain::runeclScript( std::string fileName )
{
  // TODO: autoconvert to .ecl ?
  bool exres;
  double seconds;
  size_t memory_used;
  clock_t clocks;
#ifdef _WIN32
  FILETIME dummy;
  FILETIME kernelStart, userStart;
  FILETIME kernelEnd, userEnd;
#endif
  Executor exe;
  exe.addModule( new BasicExecutorModule( exe ) );
  exe.addModule( new BasicIoExecutorModule( exe ) );
  exe.addModule( new MathExecutorModule( exe ) );
  exe.addModule( new UtilExecutorModule( exe ) );
  exe.addModule( new FileAccessExecutorModule( exe ) );
  exe.addModule( new ConfigFileExecutorModule( exe ) );
  exe.addModule( new DataFileExecutorModule( exe ) );

  ref_ptr<EScriptProgram> program( new EScriptProgram );
  if ( program->read( fileName.c_str() ) )
  {
    ERROR_PRINT << "Error reading " << fileName << "\n";
    return 1;
  }
  exe.setProgram( program.get() );
  // find and set pkg
  std::string dir = fileName;
  Clib::strip_one( dir );
  dir = Clib::normalized_dir_form( dir );
  Plib::load_packages( true /*quiet*/ );

  const auto& pkgs = Plib::systemstate.packages;
  auto pkg = std::find_if( pkgs.begin(), pkgs.end(), [&dir]( Plib::Package* p ) {
    return Clib::stringicmp( p->dir(), dir ) == 0;
  } );
  if ( pkg != pkgs.end() )
  {
    program->pkg = *pkg;
  }
  Module::load_fileaccess_cfg();  // after pkg load

  exe.setDebugLevel( m_debug ? Executor::INSTRUCTIONS : Executor::NONE );
  clock_t start = clock();
#ifdef _WIN32
  GetThreadTimes( GetCurrentThread(), &dummy, &dummy, &kernelStart, &userStart );
#endif

  exres = exe.exec();

#ifdef _WIN32
  GetThreadTimes( GetCurrentThread(), &dummy, &dummy, &kernelEnd, &userEnd );
#endif
  clocks = clock() - start;
  seconds = static_cast<double>( clocks ) / CLOCKS_PER_SEC;

  memory_used = exe.sizeEstimate();

  if ( m_profile )
  {
    fmt::Writer tmp;
    tmp << "Profiling information: \n"
        << "\tEObjectImp constructions: " << eobject_imp_constructions << "\n";
    if ( eobject_imp_count )
      tmp << "\tRemaining BObjectImps: " << eobject_imp_count << "\n";
    tmp << "\tInstruction cycles: " << escript_instr_cycles << "\n"
        << "\tInnerExec calls: " << escript_execinstr_calls << "\n"
        << "\tClocks: " << clocks << " (" << seconds << " seconds)\n"
#ifdef _WIN32
        << "\tKernel Time: " << ( *(__int64*)&kernelEnd ) - ( *(__int64*)&kernelStart ) << "\n"
        << "\tUser Time:   " << ( *(__int64*)&userEnd ) - ( *(__int64*)&userStart ) << "\n"
#endif
        << "\tSpace used: " << memory_used << "\n\n"
        << "\tCycles Per Second: " << escript_instr_cycles / seconds << "\n"
        << "\tCycles Per Minute: " << 60.0 * escript_instr_cycles / seconds << "\n"
        << "\tCycles Per Hour:   " << 3600.0 * escript_instr_cycles / seconds << "\n";
#if BOBJECTIMP_DEBUG
    display_bobjectimp_instances();
#endif
#ifdef ESCRIPT_PROFILE
    tmp << "FuncName,Count,Min,Max,Sum,Avarage\n";
    for ( escript_profile_map::iterator itr = EscriptProfileMap.begin();
          itr != EscriptProfileMap.end(); ++itr )
    {
      tmp << itr->first << "," << itr->second.count << "," << itr->second.min << ","
          << itr->second.max << "," << itr->second.sum << ","
          << ( itr->second.sum / ( 1.0 * itr->second.count ) ) << "\n";
    }
#endif
    INFO_PRINT << tmp.str();
  }
  // deinit
  Core::configurationbuffer.deinitialize();
  Plib::systemstate.deinitialize();

  return exres ? 0 : 1;
}

int RunEclMain::runecl()
{
  const std::vector<std::string>& binArgs = programArgs();

  for ( size_t i = 1; i < binArgs.size(); i++ )
  {
    switch ( binArgs[i][0] )
    {
    case '/':
    case '-':
      switch ( binArgs[i][1] )
      {
      case 'd':
      case 'D':
      case 'v':
      case 'V':
      case 'q':
      case 'Q':
      case 'p':
      case 'P':
        break;
      default:
        ERROR_PRINT << "Unknown option: " << binArgs[i] << "\n";
        return 1;
      }
      break;
    default:
      return runeclScript( binArgs[i] );
    }
  }
  return 0;
}

int RunEclMain::main()
{
  Clib::Logging::global_logger->disableFileLog();

  const std::vector<std::string>& binArgs = programArgs();
  Pol::Bscript::escript_config.max_call_depth = 100;
  m_quiet = programArgsFind( "q" );
  m_debug = programArgsFind( "d" );
  m_profile = programArgsFind( "p" );

  /**********************************************
   * show copyright
   **********************************************/
  if ( !m_quiet )
  {
    double vernum = 1 + (double)( ESCRIPT_FILE_VER_CURRENT / 100.0f );
    ERROR_PRINT << "EScript Executor v" << vernum << "\n" << POL_COPYRIGHT << "\n\n";
  }

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 )
  {
    showHelp();
    return 0;  // return "okay"
  }

  /**********************************************
   * dump script
   **********************************************/
  std::string fileName;
  if ( programArgsFind( "v", &fileName ) && fileName != "" )
  {
    dumpScript( fileName );
    return 0;  // return "okay"
  }

  /**********************************************
   * execute all given scripts
   **********************************************/
  return runecl();
}
}  // namespace Clib
}  // namespace Pol

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::Clib::RunEclMain* RunEclMain = new Pol::Clib::RunEclMain();
  RunEclMain->start( argc, argv );
}
