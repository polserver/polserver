#include "ECompileMain.h"

#include <cstdio>
#include <exception>
#include <iosfwd>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "../bscript/compiler.h"
#include "../bscript/compilercfg.h"
#include "../bscript/escriptv.h"
#include "../bscript/executor.h"
#include "../bscript/executortype.h"
#include "../bscript/filefmt.h"
#include "../bscript/parser.h"
#include "../clib/Program/ProgramConfig.h"
#include "../clib/Program/ProgramMain.h"
#include "../clib/dirlist.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/mdump.h"
#include "../clib/passert.h"
#include "../clib/threadhelp.h"
#include "../clib/timer.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include <format/format.h>

namespace Pol
{
namespace ECompile
{
using namespace std;
using namespace Pol::Core;
using namespace Pol::Plib;
using namespace Pol::Bscript;

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

ECompileMain::ECompileMain() : Pol::Clib::ProgramMain() {}
ECompileMain::~ECompileMain() {}
///////////////////////////////////////////////////////////////////////////////

void ECompileMain::showHelp()
{
  INFO_PRINT << "Usage:\n"
             << "    \n"
             << "  ECOMPILE [options] filespec [filespec ...]\n"
             << "    \n"
             << "  Output is : filespec.ecl\n"
             << "  Options:\n"
             << "   Options: \n"
             << "       -a           compile *.asp pages also\n"
             << "       -A           automatically compile scripts in main and enabled packages\n"
             << "       -Au          (as '-A' but only compile updated files)\n"
             << "       -b           keep building other scripts after errors\n"
             << "       -C cfgpath   path to configuration (replaces ecompile.cfg)\n"
             << "       -d           display confusing compiler parse information\n"
             << "       -D           write dependency information\n"
             << "       -e           report error on successful compilation (used for testing)\n"
#ifdef WIN32
             << "       -Ecfgpath    set or change the ECOMPILE_CFG_PATH evironment variable\n"
#endif
             << "       -i           include intrusive debug info in .ecl file\n"
             << "       -l           generate listfile\n"
             << "       -m           don't optimize object members\n"
#ifdef WIN32
             << "       -Pdir        set or change the EM and INC files Environment Variables\n"
#endif
             << "       -q           quiet mode (suppress normal output)\n"
             << "       -r [dir]     recurse folder [from 'dir'] (defaults to current folder)\n"
             << "       -ri [dir]      (as '-r' but only compile .inc files)\n"
             << "         -t[v]      show timing/profiling information [override quiet mode]\n"
             << "         -u         compile only updated scripts (.src newer than .ecl)\n"
             << "           -f       force compile even if up-to-date\n"
             << "       -s           display summary if -q is not set\n"
             << "       -T[N]        use threaded compilation, force N threads to run\n"
             << "       -vN          verbosity level\n"
             << "       -w           display warnings\n"
             << "       -W           generate wordfile\n"
             << "       -y           treat warnings as errors\n"
             << "       -x           write external .dbg file\n"
             << "       -xt          write external .dbg.txt info file\n"
             << "\n"
             << " NOTE:\n"
             << "   If <filespec> are required after an empty -r[i] option, you MUST specify\n"
             << "   a literal [dir] of '.' (no quotes) or options will not parse correctly.\n";
}

static int s_argc;
static char** s_argv;

int debug = 0;
bool quiet = false;
bool opt_generate_wordlist = false;
bool keep_building = false;
bool verbose = false;
bool force_update = false;
bool show_timing_details = false;
bool timing_quiet_override = false;
bool expect_compile_failure = false;
bool dont_optimize_object_members = false;
std::string EmPathEnv;   // "ECOMPILE_PATH_EM=xxx"
std::string IncPathEnv;  // ECOMPILE_PATH_INC=yyy"
std::string CfgPathEnv;  // ECOMPILE_CFG_PATH=zzz"

struct Summary
{
  unsigned UpToDateScripts = 0;
  unsigned CompiledScripts = 0;
  unsigned ScriptsWithCompileErrors = 0;
  size_t ThreadCount = 0;
} summary;

void generate_wordlist()
{
  INFO_PRINT << "Writing word list to wordlist.txt\n";
  std::ofstream ofs( "wordlist.txt", std::ios::out | std::ios::trunc );
  Parser::write_words( ofs );
}

void compile_inc( const char* path )
{
  if ( !quiet )
    INFO_PRINT << "Compiling: " << path << "\n";

  Compiler C;

  C.setQuiet( !debug );
  C.setIncludeCompileMode();
  int res = C.compileFile( path );

  if ( res )
    throw std::runtime_error( "Error compiling file" );
}

/**
 * Compiles the single given file (inc, src, hsr, asp), if needed
 *
 * Takes into account compilercfg.OnlyCompileUpdatedScripts and force_update
 *
 * @param path path of the file to be compiled
 * @return TRUE if the file was compiled, FALSE otherwise (eg. the file is up-to-date)
 */
bool compile_file( const char* path )
{
  std::string fname( path );
  std::string filename_src = fname, ext( "" );

  std::string::size_type pos = fname.rfind( '.' );
  if ( pos != std::string::npos )
    ext = fname.substr( pos );

  if ( !ext.compare( ".inc" ) )
  {
    compile_inc( path );
    return true;
  }

  if ( ext.compare( ".src" ) != 0 && ext.compare( ".hsr" ) != 0 && ext.compare( ".asp" ) != 0 )
  {
    compiler_error( "Didn't find '.src', '.hsr', or '.asp' extension on source filename '", path,
                    "'!\n" );
    throw std::runtime_error( "Error in source filename" );
  }
  std::string filename_ecl = fname.replace( pos, 4, ".ecl" );
  std::string filename_lst = fname.replace( pos, 4, ".lst" );
  std::string filename_dep = fname.replace( pos, 4, ".dep" );
  std::string filename_dbg = fname.replace( pos, 4, ".dbg" );

  if ( compilercfg.OnlyCompileUpdatedScripts && !force_update )
  {
    bool all_old = true;
    unsigned int ecl_timestamp = Clib::GetFileTimestamp( filename_ecl.c_str() );
    if ( Clib::GetFileTimestamp( filename_src.c_str() ) >= ecl_timestamp )
    {
      if ( verbose )
        INFO_PRINT << filename_src << " is newer than " << filename_ecl << "\n";
      all_old = false;
    }

    if ( all_old )
    {
      std::ifstream ifs( filename_dep.c_str() );
      // if the file doesn't exist, gotta build.
      if ( ifs.is_open() )
      {
        std::string depname;
        while ( getline( ifs, depname ) )
        {
          if ( Clib::GetFileTimestamp( depname.c_str() ) >= ecl_timestamp )
          {
            if ( verbose )
              INFO_PRINT << depname << " is newer than " << filename_ecl << "\n";
            all_old = false;
            break;
          }
        }
      }
      else
      {
        if ( verbose )
          INFO_PRINT << filename_dep << " does not exist."
                     << "\n";
        all_old = false;
      }
    }
    if ( all_old )
    {
      if ( !quiet && compilercfg.DisplayUpToDateScripts )
        INFO_PRINT << filename_ecl << " is up-to-date."
                   << "\n";
      return false;
    }
  }

  {
    if ( !quiet )
      INFO_PRINT << "Compiling: " << path << "\n";

    Compiler C;

    C.setQuiet( !debug );
    int res = C.compileFile( path );

    if ( expect_compile_failure )
    {
      if ( res )  // good, it failed
      {
        if ( !quiet )
          INFO_PRINT << "Compilation failed as expected."
                     << "\n";
        return true;
      }
      else
      {
        throw std::runtime_error( "Compilation succeeded (-e indicates failure was expected)" );
      }
    }

    if ( res )
      throw std::runtime_error( "Error compiling file" );


    if ( !quiet )
      INFO_PRINT << "Writing:   " << filename_ecl << "\n";

    if ( C.write( filename_ecl.c_str() ) )
    {
      throw std::runtime_error( "Error writing output file" );
    }

    if ( compilercfg.GenerateListing )
    {
      if ( !quiet )
        INFO_PRINT << "Writing:   " << filename_lst << "\n";
      std::ofstream ofs( filename_lst.c_str() );
      C.dump( ofs );
    }
    else if ( Clib::FileExists( filename_lst.c_str() ) )
    {
      if ( !quiet )
        INFO_PRINT << "Deleting:  " << filename_lst << "\n";
      Clib::RemoveFile( filename_lst );
    }

    if ( compilercfg.GenerateDebugInfo )
    {
      if ( !quiet )
      {
        INFO_PRINT << "Writing:   " << filename_dbg << "\n";
        if ( compilercfg.GenerateDebugTextInfo )
          INFO_PRINT << "Writing:   " << filename_dbg << ".txt"
                     << "\n";
      }
      C.write_dbg( filename_dbg.c_str(), compilercfg.GenerateDebugTextInfo );
    }
    else if ( Clib::FileExists( filename_dbg.c_str() ) )
    {
      if ( !quiet )
        INFO_PRINT << "Deleting:  " << filename_dbg << "\n";
      Clib::RemoveFile( filename_dbg );
    }

    if ( compilercfg.GenerateDependencyInfo )
    {
      if ( !quiet )
        INFO_PRINT << "Writing:   " << filename_dep << "\n";
      C.writeIncludedFilenames( filename_dep.c_str() );
    }
    else if ( Clib::FileExists( filename_dep.c_str() ) )
    {
      if ( !quiet )
        INFO_PRINT << "Deleting:  " << filename_dep << "\n";
      Clib::RemoveFile( filename_dep );
    }
  }
  return true;
}

void compile_file_wrapper( const char* path )
{
  try
  {
    if ( compile_file( path ) )
      ++summary.CompiledScripts;
    else
      ++summary.UpToDateScripts;
  }
  catch ( std::exception& )
  {
    ++summary.CompiledScripts;
    ++summary.ScriptsWithCompileErrors;
    if ( !keep_building )
      throw;
  }
}

bool setting_value( const char* arg )
{
  // format of arg is -C or -C-
  if ( arg[2] == '\0' )
    return true;
  else if ( arg[2] == '-' )
    return false;
  else if ( arg[2] == '+' )
    return true;
  else
    return true;
}


int readargs( int argc, char** argv )
{
  bool unknown_opt = false;

  for ( int i = 1; i < argc; i++ )
  {
    const char* arg = argv[i];
#ifdef __linux__
    if ( arg[0] == '-' )
#else
    if ( arg[0] == '/' || arg[0] == '-' )
#endif
    {
      switch ( arg[1] )
      {
      case 'A':  // skip it at this point.
        break;

      case 'C':  // skip it at this point.
        ++i;     // and skip its parameter.
        break;

      case 'd':
        debug = 1;
        break;

      case 'D':
        compilercfg.GenerateDependencyInfo = setting_value( arg );
        break;

      case 'e':
        expect_compile_failure = true;
        break;

#ifdef WIN32
      case 'E':
      {
        std::string path = &argv[i][2];
        CfgPathEnv = "ECOMPILE_CFG_PATH=" + path;
        _putenv( CfgPathEnv.c_str() );
      }
      break;
#endif

      case 'q':
        quiet = true;
        break;

      case 'w':
        compilercfg.DisplayWarnings = setting_value( arg );
        if ( argv[i][2] == 'P' )
          compilercfg.ParanoiaWarnings = true;
        break;
      case 'y':
        compilercfg.ErrorOnWarning = setting_value( arg );
        break;

      case 'l':
        compilercfg.GenerateListing = setting_value( arg );
        break;

      case 'i':
        include_debug = 1;
        break;

      case 'r':  // -r[i] [dir] is okay
        // Only suboption allowed is '-ri' (.inc recurse)
        if ( argv[i][2] && argv[i][2] != 'i' )
        {
          // BZZZZT! *error*
          unknown_opt = true;
          break;
        }

// Only skip next parameter if it's not an option!!
#ifdef __linux__
        if ( i + 1 < argc && argv[i + 1][0] != '-' )
#else
        if ( i + 1 < argc && argv[i + 1][0] != '/' && argv[i + 1][0] != '-' )
#endif
          ++i;
        break;

      case 't':
        show_timing_details = true;
        if ( argv[i][2] == 'v' )
          timing_quiet_override = true;
        break;

      case 's':
        // show_source = true;
        compilercfg.DisplaySummary = true;
        break;

      case 'W':
        opt_generate_wordlist = true;
        break;

      case 'a':
        compilercfg.CompileAspPages = setting_value( arg );
        break;

      case 'm':
        compilercfg.OptimizeObjectMembers = false;
        break;

      case 'b':
        keep_building = true;
        break;

      case 'u':
        compilercfg.OnlyCompileUpdatedScripts = setting_value( arg );
        if ( compilercfg.OnlyCompileUpdatedScripts )
          compilercfg.GenerateDependencyInfo = true;
        break;

      case 'f':
        force_update = true;
        break;

      case 'v':
        verbose = true;
        int vlev;
        vlev = atoi( &argv[i][2] );
        if ( !vlev )
          vlev = 1;
        Compiler::setVerbosityLevel( vlev );
        break;

      case 'x':
        compilercfg.GenerateDebugInfo = setting_value( arg );
        compilercfg.GenerateDebugTextInfo = ( argv[i][2] == 't' );
        break;

#ifdef WIN32
      case 'P':
      {
        std::string path = &argv[i][2];
        EmPathEnv = "ECOMPILE_PATH_EM=" + path;
        IncPathEnv = "ECOMPILE_PATH_INC=" + path;
        _putenv( EmPathEnv.c_str() );
        _putenv( IncPathEnv.c_str() );
      }
      break;
#endif
      case 'T':
      {
        compilercfg.ThreadedCompilation = true;
        int count = atoi( &argv[i][2] );
        compilercfg.NumberOfThreads = count;
        break;
      }
      default:
        unknown_opt = true;
        break;
      }
    }

    if ( unknown_opt )
    {
      ERROR_PRINT << "Unknown option: " << argv[i] << "\n";
      return 1;
    }
  }
  return 0;
}

/**
 * Recursively compile a folder
 *
 * @param basedir Path of the folder to recurse into
 * @param files
 */
void recurse_compile( const std::string& basedir, std::vector<std::string>* files )
{
  int s_compiled, s_uptodate, s_errors;
  clock_t start, finish;

  if ( !Clib::IsDirectory( basedir.c_str() ) )
    return;

  s_compiled = s_uptodate = s_errors = 0;
  start = clock();
  for ( Clib::DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
  {
    if ( Clib::exit_signalled )
      return;
    std::string name = dl.name(), ext;
    if ( name[0] == '.' )
      continue;

    std::string::size_type pos = name.rfind( '.' );
    if ( pos != std::string::npos )
      ext = name.substr( pos );

    try
    {
      if ( pos != std::string::npos &&
           ( !ext.compare( ".src" ) || !ext.compare( ".hsr" ) ||
             ( compilercfg.CompileAspPages && !ext.compare( ".asp" ) ) ) )
      {
        s_compiled++;
        if ( files == nullptr )
        {
          if ( compile_file( ( basedir + name ).c_str() ) )
          {
            ++summary.CompiledScripts;
          }
          else
          {
            ++s_uptodate;
            ++summary.UpToDateScripts;
          }
        }
        else
          files->push_back( ( basedir + name ) );
      }
      else
      {
        recurse_compile( basedir + name + "/", files );
      }
    }
    catch ( std::exception& )
    {
      ++summary.CompiledScripts;
      ++summary.ScriptsWithCompileErrors;
      if ( !keep_building )
        throw;
      s_errors++;
    }
  }
  if ( files == nullptr )
    return;
  finish = clock();

  if ( ( !quiet || timing_quiet_override ) && show_timing_details && s_compiled > 0 &&
       files == nullptr )
  {
    INFO_PRINT << "Compiled " << s_compiled << " script" << ( s_compiled == 1 ? "" : "s" ) << " in "
               << basedir << " in " << (int)( ( finish - start ) / CLOCKS_PER_SEC )
               << " second(s)\n";
    if ( s_uptodate > 0 )
      INFO_PRINT << "    " << s_uptodate << " script" << ( s_uptodate == 1 ? " was" : "s were" )
                 << " already up-to-date.\n";
    if ( s_errors > 0 )
      INFO_PRINT << "    " << s_errors << " script" << ( s_errors == 1 ? "" : "s" )
                 << " had errors.\n";
  }
}
void recurse_compile_inc( const std::string& basedir, std::vector<std::string>* files )
{
  for ( Clib::DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
  {
    if ( Clib::exit_signalled )
      return;
    std::string name = dl.name(), ext;
    if ( name[0] == '.' )
      continue;

    std::string::size_type pos = name.rfind( '.' );
    if ( pos != std::string::npos )
      ext = name.substr( pos );

    if ( pos != std::string::npos && !ext.compare( ".inc" ) )
    {
      if ( files == nullptr )
        compile_file( ( basedir + name ).c_str() );
      else
        files->push_back( ( basedir + name ) );
    }
    else
    {
      recurse_compile( basedir + name + "/", files );
    }
  }
}

void parallel_compile( const std::vector<std::string>& files )
{
  std::atomic<unsigned> compiled_scripts( 0 );
  std::atomic<unsigned> uptodate_scripts( 0 );
  std::atomic<unsigned> error_scripts( 0 );
  std::atomic<bool> par_keep_building( true );
  {
    unsigned int thread_count = std::max( 2u, std::thread::hardware_concurrency() * 2 );
    if ( compilercfg.NumberOfThreads )
      thread_count = static_cast<unsigned>( compilercfg.NumberOfThreads );
    threadhelp::TaskThreadPool pool( thread_count, "ecompile" );
    summary.ThreadCount = pool.size();
    for ( const auto& file : files )
    {
      pool.push( [&]() {
        if ( !par_keep_building || Clib::exit_signalled )
          return;
        try
        {
          if ( compile_file( file.c_str() ) )
            ++compiled_scripts;
          else
            ++uptodate_scripts;
        }
        catch ( std::exception& e )
        {
          ++compiled_scripts;
          ++error_scripts;
          compiler_error( "failed to compile ", file.c_str(), ": ", e.what(), "\n" );
          if ( !keep_building )
          {
            par_keep_building = false;
          }
        }
        catch ( ... )
        {
          par_keep_building = false;
          Clib::force_backtrace();
        }
      } );
    }
  }
  summary.CompiledScripts = compiled_scripts;
  summary.UpToDateScripts = uptodate_scripts;
  summary.ScriptsWithCompileErrors = error_scripts;
}


/**
 * Runs the compilation threads
 */
void AutoCompile()
{
  bool save = compilercfg.OnlyCompileUpdatedScripts;
  compilercfg.OnlyCompileUpdatedScripts = compilercfg.UpdateOnlyOnAutoCompile;
  if ( compilercfg.ThreadedCompilation )
  {
    std::vector<std::string> files;
    recurse_compile( Clib::normalized_dir_form( compilercfg.PolScriptRoot ), &files );
    for ( const auto& pkg : Plib::systemstate.packages )
    {
      recurse_compile( Clib::normalized_dir_form( pkg->dir() ), &files );
    }
    parallel_compile( files );
  }
  else
  {
    recurse_compile( Clib::normalized_dir_form( compilercfg.PolScriptRoot ), nullptr );
    for ( const auto& pkg : Plib::systemstate.packages )
    {
      recurse_compile( Clib::normalized_dir_form( pkg->dir() ), nullptr );
    }
  }
  compilercfg.OnlyCompileUpdatedScripts = save;
}

/**
 * Takes decisions, runs, the compilation, prints summary and cleans after
 */
bool run( int argc, char** argv, int* res )
{
  Clib::enable_exit_signaller();
  // Load and analyze the package structure
  for ( const auto& elem : compilercfg.PackageRoot )
  {
    Plib::load_packages( elem, true /* quiet */ );
  }
  Plib::replace_packages();
  Plib::check_package_deps();

  // Determine the run mode and do the compile itself
  Tools::Timer<> timer;
  bool any = false;

  for ( int i = 1; i < argc; i++ )
  {
#ifdef __linux__
    if ( argv[i][0] == '-' )
#else
    if ( argv[i][0] == '/' || argv[i][0] == '-' )
#endif
    {
      // -r[i] [<dir>]
      if ( argv[i][1] == 'A' )
      {
        compilercfg.UpdateOnlyOnAutoCompile = ( argv[i][2] == 'u' );
        any = true;

        AutoCompile();
      }
      else if ( argv[i][1] == 'r' )
      {
        any = true;
        std::string dir( "." );
        bool compile_inc = ( argv[i][2] == 'i' );  // compile .inc files

        ++i;
        if ( i < argc && argv[i] && argv[i][0] != '-' )
          dir.assign( argv[i] );

        if ( compilercfg.ThreadedCompilation )
        {
          std::vector<std::string> files;
          if ( compile_inc )
            recurse_compile_inc( Clib::normalized_dir_form( dir ), &files );
          else
            recurse_compile( Clib::normalized_dir_form( dir ), &files );
          parallel_compile( files );
        }
        else
        {
          if ( compile_inc )
            recurse_compile_inc( Clib::normalized_dir_form( dir ), nullptr );
          else
            recurse_compile( Clib::normalized_dir_form( dir ), nullptr );
        }
      }
      else if ( argv[i][1] == 'C' )
      {
        ++i;  // skip the config file pathname
      }
      // and skip any other option.
    }
    else
    {
      any = true;
#ifdef _WIN32
      Clib::forspec( argv[i], compile_file_wrapper );
#else
      compile_file_wrapper( argv[i] );
#endif
    }
  }

  if ( !any && compilercfg.AutoCompileByDefault )
  {
    any = true;
    AutoCompile();
  }

  // Execution is completed: start final/cleanup tasks
  timer.stop();

  Plib::systemstate.deinitialize();

  if ( any && compilercfg.DisplaySummary && !quiet )
  {
    fmt::Writer tmp;
    tmp << "Compilation Summary:\n";
    if ( summary.ThreadCount )
      tmp << "    Used " << summary.ThreadCount << " threads\n";
    if ( summary.CompiledScripts )
      tmp << "    Compiled " << summary.CompiledScripts << " script"
          << ( summary.CompiledScripts == 1 ? "" : "s" ) << " in " << timer.ellapsed() << " ms.\n";

    if ( summary.ScriptsWithCompileErrors )
      tmp << "    " << summary.ScriptsWithCompileErrors << " of those script"
          << ( summary.ScriptsWithCompileErrors == 1 ? "" : "s" ) << " had errors.\n";

    if ( summary.UpToDateScripts )
      tmp << "    " << summary.UpToDateScripts << " script"
          << ( summary.UpToDateScripts == 1 ? " was" : "s were" ) << " already up-to-date.\n";
    INFO_PRINT << tmp.str();
  }

  if ( summary.ScriptsWithCompileErrors )
    *res = 1;
  else
    *res = 0;
  return any;
}

void read_config_file( int argc, char* argv[] )
{
  for ( int i = 1; i < argc; i++ )
  {
    if ( argv[i][0] == '/' || argv[i][0] == '-' )
    {
      // -C cfgpath
      if ( argv[i][1] == 'C' )
      {
        ++i;
        if ( i == argc )
          throw std::runtime_error( "-C specified without pathname" );

        compilercfg.Read( std::string( argv[i] ) );
        return;
      }
    }
  }

  // check ECOMPILE_CFG_PATH environment variable
  const char* env_ecompile_cfg_path = getenv( "ECOMPILE_CFG_PATH" );
  if ( env_ecompile_cfg_path != nullptr )
  {
    compilercfg.Read( std::string( env_ecompile_cfg_path ) );
    return;
  }

  // no -C arg, so use binary path (hope it's right..sigh.)
  std::string cfgpath = PROG_CONFIG::programDir() + "ecompile.cfg";
  if ( Clib::FileExists( "ecompile.cfg" ) )
  {
    compilercfg.Read( "ecompile.cfg" );
  }
  else if ( Clib::FileExists( cfgpath ) )
  {
    compilercfg.Read( cfgpath );
  }
  else
  {
    ERROR_PRINT << "Could not find " << cfgpath << "; using defaults.\n";
    compilercfg.SetDefaults();
  }
}

/**
 * This is the main entry point for ecompile program
 */
int ECompileMain::main()
{
  Clib::Logging::global_logger->disableFileLog();

  const std::vector<std::string>& binArgs = programArgs();

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 )
  {
    showHelp();
    return 0;  // return "okay"
  }

/**********************************************
 * TODO: rework the following cruft from former uotool.cpp
 **********************************************/
#ifdef _WIN32
  Clib::MiniDumper::Initialize();
#endif

  ECompile::read_config_file( s_argc, s_argv );

  int res = ECompile::readargs( s_argc, s_argv );
  if ( res )
  {
    showHelp();
    return res;
  }

  if ( !ECompile::quiet )
  {
    // vX.YY
    double vernum = (double)1 + (double)( ESCRIPT_FILE_VER_CURRENT / 100.0f );
    INFO_PRINT << "EScript Compiler v" << vernum << "\n" << POL_COPYRIGHT << "\n\n";
  }

  if ( ECompile::opt_generate_wordlist )
  {
    ECompile::generate_wordlist();
    return 0;
  }
  int prog_res = 1;
  bool didanything = ECompile::run( s_argc, s_argv, &prog_res );

  if ( !didanything )
  {
    showHelp();
    return 1;
  }
  return prog_res;
}
}  // namespace ECompile
}  // namespace Pol

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::ECompile::s_argc = argc;
  Pol::ECompile::s_argv = argv;

  Pol::ECompile::ECompileMain* ECompileMain = new Pol::ECompile::ECompileMain();
  ECompileMain->start( argc, argv );
}
