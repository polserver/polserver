#include "ecompile/ECompileMain.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <system_error>
#include <time.h>
#include <vector>

#include <fmt/std.h>

#include "ecompile/EfswFileWatchListener.h"
#include "ecompile/ProgressIndicator.h"

#include "bscript/compiler/Compiler.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compilercfg.h"
#include "bscript/escriptv.h"
#include "bscript/executor.h"
#include "bscript/executortype.h"
#include "bscript/filefmt.h"
#include "clib/Program/ProgramConfig.h"
#include "clib/Program/ProgramMain.h"
#include "clib/clib.h"
#include "clib/esignal.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/mdump.h"
#include "clib/passert.h"
#include "clib/threadhelp.h"
#include "clib/timer.h"
#include "plib/pkg.h"
#include "plib/systemstate.h"

#define VERBOSE_PRINTLN                 \
  if ( compilercfg.VerbosityLevel > 0 ) \
  INFO_PRINTLN


namespace Pol::ECompile
{
namespace fs = std::filesystem;
using namespace Pol::Core;
using namespace Pol::Plib;
using namespace Pol::Bscript;
using namespace std::chrono_literals;

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

ECompileMain::ECompileMain() : Pol::Clib::ProgramMain() {}
ECompileMain::~ECompileMain() = default;
///////////////////////////////////////////////////////////////////////////////

void ECompileMain::showHelp()
{
  INFO_PRINTLN(
      "Usage:\n"
      "    \n"
      "  ECOMPILE [options] filespec [filespec ...]\n"
      "    \n"
      "  Output is : filespec.ecl\n"
      "  Options:\n"
      "   Options: \n"
      "       -F           format filespec (print result)\n"
      "       -Fi          format filespec (inplace)\n"
      "       -Z           generate abstract syntax tree (AST)\n"
      "       -a           compile *.asp pages also\n"
      "       -A           automatically compile scripts in main and enabled packages\n"
      "       -Au          (as '-A' but only compile updated files)\n"
      "       -b           keep building other scripts after errors\n"
      "       -c           treat wrong capitalization in include directives as error\n"
      "       -C cfgpath   path to configuration (replaces ecompile.cfg)\n"
      "       -d           display confusing compiler parse information\n"
      "       -D           write dependency information\n"
      "       -e           report error on successful compilation (used for testing)\n"
#ifdef WIN32
      "       -Ecfgpath    set or change the ECOMPILE_CFG_PATH evironment variable\n"
#endif

      "       -i           include intrusive debug info in .ecl file\n"
      "       -l           generate listfile\n"
      "       -m           don't optimize object members\n"
#ifdef WIN32
      "       -Pdir        set or change the EM and INC files Environment Variables\n"
#endif
      "       -p           progress mode: replace per-script output with one line that\n"
      "                    updates in place, and show the summary at the end. The count\n"
      "                    covers every script considered, including up-to-date ones.\n"
      "       -q           quiet mode (suppress normal output)\n"
      "       -r [dir]     recurse folder [from 'dir'] (defaults to current folder)\n"
      "       -ri [dir]    (as '-r' but only compile .inc files)\n"
      "       -t[v]        show timing/profiling information [override quiet mode]\n"
      "       -u           compile only updated scripts (.src newer than .ecl)\n"
      "       -f           force compile even if up-to-date\n"
      "       -s           display summary if -q is not set\n"
      "       -T[N]        use threaded compilation, force N threads to run\n"
      "       -vN          verbosity level\n"
      "       -w           display warnings\n"
      "       -W           watch mode\n"
      "       -y           treat warnings as errors\n"
      "       -x           write external .dbg file\n"
      "       -xt          write external .dbg.txt info file\n"
      "       -S           perform short-circuit evaluation\n"
      "\n"
      " NOTE:\n"
      "   If <filespec> are required after an empty -r[i] option, you MUST specify\n"
      "   a literal [dir] of '.' (no quotes) or options will not parse correctly." );
}
static int s_argc;
static char** s_argv;

int debug = 0;
bool quiet = false;
bool progress_mode = false;
bool keep_building = false;
bool force_update = false;
bool format_source = false;
bool format_source_inplace = false;
bool show_timing_details = false;
bool timing_quiet_override = false;
bool expect_compile_failure = false;
bool dont_optimize_object_members = false;
bool watch_source = false;
std::string EmPathEnv;   // "ECOMPILE_PATH_EM=xxx"
std::string IncPathEnv;  // ECOMPILE_PATH_INC=yyy"
std::string CfgPathEnv;  // ECOMPILE_CFG_PATH=zzz"

struct Summary
{
  unsigned UpToDateScripts = 0;
  unsigned CompiledScripts = 0;
  unsigned ScriptsWithCompileErrors = 0;
  // Written directly from the compile worker threads, unlike the counters above, which the
  // threaded path accumulates locally and folds in once the pool has drained.
  std::atomic<unsigned> TotalErrors{ 0 };
  std::atomic<unsigned> TotalWarnings{ 0 };
  size_t ThreadCount = 0;
  Compiler::Profile profile;
} summary;

std::unique_ptr<ProgressIndicator> progress;

Compiler::SourceFileCache em_parse_tree_cache( summary.profile );
Compiler::SourceFileCache inc_parse_tree_cache( summary.profile );

using DependencyInfo = std::map<fs::path, std::set<fs::path>>;
// Map owner (sources) -> dependencies
DependencyInfo owner_dependencies;
// Map dependency -> owners (sources)
DependencyInfo dependency_owners;

std::set<fs::path> compiled_dirs;

std::unique_ptr<Compiler::Compiler> create_compiler()
{
  auto compiler = std::make_unique<Compiler::Compiler>( em_parse_tree_cache, inc_parse_tree_cache,
                                                        summary.profile );
  return compiler;
}

// What the compiler reported for the file this thread has in hand. A file that fails before
// reaching the compiler -- a filespec that is not a script, an .ecl that cannot be written --
// still owes the run one error.
thread_local unsigned file_error_count = 0;

void accumulate_report_counts( const Compiler::Compiler& compiler )
{
  file_error_count += compiler.error_count();
  summary.TotalErrors += compiler.error_count();
  summary.TotalWarnings += compiler.warning_count();
}

void count_failed_file()
{
  if ( !file_error_count )
    ++summary.TotalErrors;
}

/// Whether to print the per-script lines: -q drops them, -p replaces them with one line that
/// is redrawn in place.
bool show_script_chatter()
{
  return !quiet && !progress_mode;
}

void load_packages()
{
  // No need to load packages if only formatting sources
  if ( !format_source )
  {
    // Load and analyze the package structure
    for ( const auto& elem : compilercfg.PackageRoot )
    {
      Plib::load_packages( elem, true /* quiet */ );
    }
    Plib::replace_packages();
    Plib::check_package_deps();
  }
}

void compile_inc( const std::string& path )
{
  if ( show_script_chatter() )
    INFO_PRINTLN( "Compiling: {}", path );

  std::unique_ptr<Compiler::Compiler> compiler = create_compiler();

  compiler->set_include_compile_mode();
  bool res = compiler->compile_file( path );
  accumulate_report_counts( *compiler );

  if ( !res )
    throw std::runtime_error( "Error compiling file" );
}

bool format_file( const std::string& path )
{
  std::string ext( "" );

  std::string::size_type pos = path.rfind( '.' );
  if ( pos != std::string::npos )
    ext = path.substr( pos );

  if ( ext.compare( ".src" ) != 0 && ext.compare( ".inc" ) != 0 && ext.compare( ".em" ) != 0 )
  {
    ERROR_PRINTLN(
        "Didn't find '.src', '.inc', or '.em' extension on source filename '{}'! ..Ignoring",
        path );
    return true;
  }

  if ( show_script_chatter() )
    INFO_PRINTLN( "Formatting: {}", path );

  std::unique_ptr<Compiler::Compiler> compiler = create_compiler();

  bool success =
      compiler->format_file( path.c_str(), ext.compare( ".em" ) == 0, format_source_inplace );

  if ( expect_compile_failure )
  {
    if ( !success )  // good, it failed
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Formatting failed as expected." );
      return true;
    }

    throw std::runtime_error( "Formatting succeeded (-e indicates failure was expected)" );
  }

  // Not before the -e check: a failure the run asked for is not one of the run's errors.
  accumulate_report_counts( *compiler );

  if ( !success )
    throw std::runtime_error( "Error formatting file" );
  return true;
}

void add_dependency_info( const fs::path& filepath_src,
                          std::set<fs::path>* removed_dependencies = nullptr,
                          std::set<fs::path>* new_dependencies = nullptr )
{
  auto filename_dep = fs::path( filepath_src ).replace_extension( ".dep" );

  auto& dependencies = owner_dependencies[filepath_src];
  auto previous_dependencies = dependencies;

  VERBOSE_PRINTLN( "Dependencies for: {}", filepath_src );
  for ( const auto& dependency : previous_dependencies )
  {
    if ( auto itr = dependency_owners.find( dependency ); itr != dependency_owners.end() )
    {
      // Remove deleted file from owners
      itr->second.erase( filepath_src );
      auto remaining = itr->second.size();
      VERBOSE_PRINTLN( "  - Removed {}, remaining owners={}", dependency, remaining );
      if ( remaining == 0 )
      {
        dependency_owners.erase( itr );
      }
    }
  }

  dependencies.clear();

  {
    std::ifstream ifs( filename_dep.c_str() );
    if ( ifs.is_open() )
    {
      std::string depname;
      while ( getline( ifs, depname ) )
      {
        fs::path depnamepath = fs::canonical( fs::path( depname ) );
        auto& owners = dependency_owners[depnamepath];
        // Add this source as a dependency by:
        // (1) placing `filename_src` in the set of owners for `depnamepath`.
        owners.emplace( filepath_src );
        // (2) placing `depnamepath` in the set of dependencies for this `filename_src`.
        dependencies.emplace( depnamepath );
        VERBOSE_PRINTLN( "  - Added {}, owners={}", depnamepath, owners.size() );
      }
    }

    // Could not load dependency information -- maybe failed compilation? We
    // still add this source as a dependency of itself, such that it will be
    // recompiled on next save.
    //
    // On failed compilation, the existing `.dep`
    // remains, so we will use that information for recompilation (eg. a source
    // failed to compile due to a bad include, changing the include should still
    // recompile the source.)
    else
    {
      dependency_owners[filepath_src] = std::set<fs::path>{ filepath_src };
      dependencies.emplace( filepath_src );
    }
  }

  if ( removed_dependencies && new_dependencies )
  {
    std::set_difference( previous_dependencies.begin(), previous_dependencies.end(),
                         dependencies.begin(), dependencies.end(),
                         std::inserter( *removed_dependencies, removed_dependencies->begin() ) );


    std::set_difference( dependencies.begin(), dependencies.end(), previous_dependencies.begin(),
                         previous_dependencies.end(),
                         std::inserter( *new_dependencies, new_dependencies->begin() ) );
  }
}

/**
 * Compiles the single given file (inc, src, hsr, asp), if needed
 *
 * Takes into account compilercfg.OnlyCompileUpdatedScripts and force_update
 *
 * @param path path of the file to be compiled
 * @return TRUE if the file was compiled, FALSE otherwise (eg. the file is up-to-date)
 */
bool compile_file( const std::string& path )
{
  std::string ext( "" );

  std::string::size_type pos = path.rfind( '.' );
  if ( pos != std::string::npos )
    ext = path.substr( pos );

  if ( !ext.compare( ".inc" ) )
  {
    compile_inc( path );
    return true;
  }

  if ( ext.compare( ".src" ) != 0 && ext.compare( ".hsr" ) != 0 && ext.compare( ".asp" ) != 0 )
  {
    ERROR_PRINTLN( "Didn't find '.src', '.hsr', or '.asp' extension on source filename '{}'!",
                   path );
    throw std::runtime_error( "Error in source filename" );
  }
  std::string fname = path;
  std::string filename_ecl = fname.replace( pos, 4, ".ecl" );
  std::string filename_lst = fname.replace( pos, 4, ".lst" );
  std::string filename_ast = fname.replace( pos, 4, ".ast" );
  std::string filename_dep = fname.replace( pos, 4, ".dep" );
  std::string filename_dbg = fname.replace( pos, 4, ".dbg" );

  if ( compilercfg.OnlyCompileUpdatedScripts && !force_update )
  {
    bool all_old = true;
    unsigned int ecl_timestamp = Clib::GetFileTimestamp( filename_ecl.c_str() );
    if ( Clib::GetFileTimestamp( path.c_str() ) >= ecl_timestamp )
    {
      if ( compilercfg.VerbosityLevel > 0 )
        INFO_PRINTLN( "{} is newer than {}", path, filename_ecl );
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
            if ( compilercfg.VerbosityLevel > 0 )
              INFO_PRINTLN( "{} is newer than {}", depname, filename_ecl );
            all_old = false;
            break;
          }
        }
      }
      else
      {
        if ( compilercfg.VerbosityLevel > 0 )
          INFO_PRINTLN( "{} does not exist.", filename_dep );
        all_old = false;
      }
    }
    if ( all_old )
    {
      if ( show_script_chatter() && compilercfg.DisplayUpToDateScripts )
        INFO_PRINTLN( "{} is up-to-date.", filename_ecl );
      return false;
    }
  }


  {
    if ( show_script_chatter() )
      INFO_PRINTLN( "Compiling: {}", path );

    std::unique_ptr<Compiler::Compiler> compiler = create_compiler();

    bool success = compiler->compile_file( path.c_str() );

    em_parse_tree_cache.keep_some();
    inc_parse_tree_cache.keep_some();

    if ( expect_compile_failure )
    {
      if ( !success )  // good, it failed
      {
        if ( show_script_chatter() )
          INFO_PRINTLN( "Compilation failed as expected." );
        return true;
      }

      throw std::runtime_error( "Compilation succeeded (-e indicates failure was expected)" );
    }

    // Not before the -e check: a failure the run asked for is not one of the run's errors.
    accumulate_report_counts( *compiler );

    if ( !success )
      throw std::runtime_error( "Error compiling file" );


    if ( show_script_chatter() )
      INFO_PRINTLN( "Writing:   {}", filename_ecl );

    if ( !compiler->write_ecl( filename_ecl ) )
    {
      throw std::runtime_error( "Error writing output file" );
    }

    if ( compilercfg.GenerateListing )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Writing:   {}", filename_lst );
      compiler->write_listing( filename_lst );
    }
    else if ( Clib::FileExists( filename_lst.c_str() ) )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Deleting:  {}", filename_lst );
      Clib::RemoveFile( filename_lst );
    }

    if ( compilercfg.GenerateAbstractSyntaxTree )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Writing:   {}", filename_ast );
      compiler->write_string_tree( filename_ast );
    }
    else if ( Clib::FileExists( filename_ast.c_str() ) )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Deleting:  {}", filename_ast );
      Clib::RemoveFile( filename_ast );
    }

    if ( compilercfg.GenerateDebugInfo )
    {
      if ( show_script_chatter() )
      {
        INFO_PRINTLN( "Writing:   {}", filename_dbg );
        if ( compilercfg.GenerateDebugTextInfo )
          INFO_PRINTLN( "Writing:   {}.txt", filename_dbg );
      }
      compiler->write_dbg( filename_dbg, compilercfg.GenerateDebugTextInfo );
    }
    else if ( Clib::FileExists( filename_dbg.c_str() ) )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Deleting:  {}", filename_dbg );
      Clib::RemoveFile( filename_dbg );
    }

    if ( compilercfg.GenerateDependencyInfo )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Writing:   {}", filename_dep );
      compiler->write_included_filenames( filename_dep );
    }
    else if ( Clib::FileExists( filename_dep.c_str() ) )
    {
      if ( show_script_chatter() )
        INFO_PRINTLN( "Deleting:  {}", filename_dep );
      Clib::RemoveFile( filename_dep );
    }
  }
  return true;
}

bool process_file( const std::string& path )
{
  file_error_count = 0;
  if ( format_source )
    return format_file( path );
  return compile_file( path );
}

void process_file_wrapper( const std::string& path,
                           std::set<fs::path>* removed_dependencies = nullptr,
                           std::set<fs::path>* new_dependencies = nullptr )
{
  try
  {
    if ( process_file( path ) )
      ++summary.CompiledScripts;
    else
      ++summary.UpToDateScripts;
  }
  catch ( std::exception& )
  {
    ++summary.CompiledScripts;
    ++summary.ScriptsWithCompileErrors;
    count_failed_file();
    if ( !keep_building )
      throw;
  }

  if ( watch_source )
  {
    fs::path filepath = fs::canonical( fs::path( path ) );
    auto ext = filepath.extension().generic_string();
    if ( ext.compare( ".src" ) == 0 || ext.compare( ".hsr" ) == 0 || ext.compare( ".asp" ) == 0 )
    {
      add_dependency_info( filepath, removed_dependencies, new_dependencies );
    }
  }
}

bool setting_value( const char* arg )
{
  // format of arg is -C or -C-
  if ( arg[2] == '\0' )
    return true;
  if ( arg[2] == '-' )
    return false;
  if ( arg[2] == '+' )
    return true;
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
      case 'W':
        watch_source = true;
        compilercfg.GenerateDependencyInfo = true;
        keep_building = true;
        compilercfg.ThreadedCompilation =
            false;  // limitation since dependency gathering is not thread-safe
        break;

      case 'A':  // skip it at this point.
        break;

      case 'c':
        compilercfg.ErrorOnFileCaseMissmatch = setting_value( arg );
        break;

      case 'C':  // skip it at this point.
        ++i;     // and skip its parameter.
        break;

      case 'd':
        compilercfg.DisplayDebugs = setting_value( arg );
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

      case 'g':
      {
        auto value = setting_value( arg );
        if ( !value )
        {
          INFO_PRINTLN( "The OG Compiler has been removed." );
          unknown_opt = true;
        }
        break;
      }

      case 'p':
        progress_mode = true;
        compilercfg.DisplayFileOutcome = false;
        compilercfg.DisplaySummary = true;
        break;

      case 'q':
        quiet = true;
        compilercfg.DisplayFileOutcome = false;
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

      case 'F':
      {
        if ( argv[i][2] && argv[i][2] == 'i' )
          format_source_inplace = true;
        format_source = true;
        break;
      }

      case 'f':
        force_update = true;
        break;

      case 'v':
        int vlev;
        vlev = atoi( &argv[i][2] );
        if ( !vlev )
          vlev = 1;
        compilercfg.VerbosityLevel = vlev;
        break;

      case 'x':
        compilercfg.GenerateDebugInfo = setting_value( arg );
        compilercfg.GenerateDebugTextInfo = ( argv[i][2] == 't' );
        break;

      case 'Z':
        compilercfg.GenerateAbstractSyntaxTree = setting_value( arg );
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
      case 'S':
        compilercfg.ShortCircuitEvaluation = setting_value( arg );
        break;
      default:
        unknown_opt = true;
        break;
      }
    }

    if ( unknown_opt )
    {
      ERROR_PRINTLN( "Unknown option: {}", argv[i] );
      return 1;
    }
  }
  return 0;
}

void apply_configuration()
{
  em_parse_tree_cache.configure( compilercfg.EmParseTreeCacheSize );
  inc_parse_tree_cache.configure( compilercfg.IncParseTreeCacheSize );
}

/// Walks the trees and answers what would be compiled, so the run knows its total up front.
std::vector<std::string> collect_files( const std::vector<fs::path>& basedirs, bool inc_files )
{
  std::vector<std::string> result;
  std::set<std::string> seen;
  for ( const auto& basedir : basedirs )
  {
    if ( !fs::is_directory( basedir ) )
      continue;
    std::error_code ec;
    for ( auto dir_itr = fs::recursive_directory_iterator( basedir, ec );
          dir_itr != fs::recursive_directory_iterator(); ++dir_itr )
    {
      if ( Clib::exit_signalled )
        return result;
      if ( auto fn = dir_itr->path().filename().string(); !fn.empty() && *fn.begin() == '.' )
      {
        if ( dir_itr->is_directory() )
          dir_itr.disable_recursion_pending();
        continue;
      }
      if ( !dir_itr->is_regular_file() )
        continue;
      const auto ext = dir_itr->path().extension();
      const auto file = dir_itr->path().generic_string();
      if ( inc_files )
      {
        if ( !ext.compare( ".inc" ) )
          if ( seen.insert( file ).second )
            result.push_back( file );
      }
      else if ( !ext.compare( ".src" ) || !ext.compare( ".hsr" ) ||
                ( compilercfg.CompileAspPages && !ext.compare( ".asp" ) ) )
      {
        if ( seen.insert( file ).second )
          result.push_back( file );
      }
    }
  }
  return result;
}

void process_files( const std::vector<std::string>& files )
{
  if ( files.empty() )
    return;

  if ( !compilercfg.ThreadedCompilation )
  {
    for ( const auto& file : files )
    {
      if ( Clib::exit_signalled )
        return;
      progress->advance( file );
      process_file_wrapper( file );
    }
    return;
  }
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
    for ( const auto& queued : files )
    {
      pool.push(
          [&, file = queued]()
          {
            if ( !par_keep_building || Clib::exit_signalled )
              return;
            progress->advance( file );
            try
            {
              if ( process_file( file ) )
                ++compiled_scripts;
              else
                ++uptodate_scripts;
            }
            catch ( std::exception& e )
            {
              ++compiled_scripts;
              ++error_scripts;
              count_failed_file();
              ERROR_PRINTLN( "failed to compile {}: {}", file, e.what() );
              if ( !keep_building )
                par_keep_building = false;
            }
            catch ( ... )
            {
              par_keep_building = false;
              Clib::force_backtrace();
            }
          } );
    }
  }
  // Accumulate: one command line can name several trees, and each gets its own pool.
  summary.CompiledScripts += compiled_scripts;
  summary.UpToDateScripts += uptodate_scripts;
  summary.ScriptsWithCompileErrors += error_scripts;
}

void DisplaySummary( const Tools::Timer<>& timer )
{
  std::string tmp = "Compilation Summary:\n";
  if ( summary.ThreadCount )
    tmp += fmt::format( "    Used {} threads\n", summary.ThreadCount );
  if ( summary.CompiledScripts )
    tmp += fmt::format( "    Compiled {} script{} in {} ms.\n", summary.CompiledScripts,
                        ( summary.CompiledScripts == 1 ? "" : "s" ), timer.ellapsed() );

  if ( summary.ScriptsWithCompileErrors )
    tmp += fmt::format( "    {} of those script{} had errors.\n", summary.ScriptsWithCompileErrors,
                        ( summary.ScriptsWithCompileErrors == 1 ? "" : "s" ) );

  if ( summary.UpToDateScripts )
    tmp += fmt::format( "    {} script{} already up-to-date.\n", summary.UpToDateScripts,
                        ( summary.UpToDateScripts == 1 ? " was" : "s were" ) );

  {
    auto errors = summary.TotalErrors.load();
    tmp += fmt::format( "    {} error{}", errors, ( errors == 1 ? "" : "s" ) );
    // Warnings are counted whether or not they were displayed, so only claim a total the user
    // could have seen.
    if ( compilercfg.DisplayWarnings || compilercfg.ErrorOnWarning )
    {
      auto warnings = summary.TotalWarnings.load();
      tmp += fmt::format( ", {} warning{}", warnings, ( warnings == 1 ? "" : "s" ) );
    }
    tmp += ".\n";
  }

  if ( show_timing_details )
  {
    tmp += fmt::format( "    build workspace: {}\n",
                        (long long)summary.profile.build_workspace_micros / 1000 );
    tmp += fmt::format( "        - load *.em:   {}\n",
                        (long long)summary.profile.load_em_micros / 1000 );
    tmp += fmt::format( "       - parse *.em:   {} ({})\n",
                        (long long)summary.profile.parse_em_micros / 1000,
                        (long)summary.profile.parse_em_count );
    tmp += fmt::format( "         - ast *.em:   {}\n",
                        (long long)summary.profile.ast_em_micros / 1000 );
    tmp += fmt::format( "      - parse *.inc:   {} ({})\n",
                        (long long)summary.profile.parse_inc_micros / 1000,
                        (long)summary.profile.parse_inc_count );
    tmp += fmt::format( "        - ast *.inc:   {}\n",
                        (long long)summary.profile.ast_inc_micros / 1000 );
    tmp += fmt::format( "      - parse *.src:   {} ({})\n",
                        (long long)summary.profile.parse_src_micros / 1000,
                        (long)summary.profile.parse_src_count );
    tmp += fmt::format( "        - ast *.src:   {}\n",
                        (long long)summary.profile.ast_src_micros / 1000 );
    tmp += fmt::format( "  resolve functions:   {}\n",
                        (long long)summary.profile.ast_resolve_functions_micros / 1000 );
    tmp += fmt::format( " register constants: {}\n",
                        (long long)summary.profile.register_const_declarations_micros / 1000 );
    tmp += fmt::format( "            analyze: {}\n",
                        (long long)summary.profile.analyze_micros / 1000 );
    tmp += fmt::format( "           optimize: {}\n",
                        (long long)summary.profile.optimize_micros / 1000 );
    tmp += fmt::format( "       disambiguate: {}\n",
                        (long long)summary.profile.disambiguate_micros / 1000 );
    tmp += fmt::format( "      generate code: {}\n",
                        (long long)summary.profile.codegen_micros / 1000 );
    tmp += fmt::format( "  prune cache (sel): {}\n",
                        (long long)summary.profile.prune_cache_select_micros / 1000 );
    tmp += fmt::format( "  prune cache (del): {}\n",
                        (long long)summary.profile.prune_cache_delete_micros / 1000 );
    tmp += "\n";
    tmp += fmt::format( "      - ambiguities: {}\n", (long)summary.profile.ambiguities );
    tmp += fmt::format( "       - cache hits: {}\n", (long)summary.profile.cache_hits );
    tmp += fmt::format( "     - cache misses: {}\n", (long)summary.profile.cache_misses );
  }

  INFO_PRINTLN( tmp );
}

void EnterWatchMode()
{
  std::list<WatchFileMessage> watch_messages;
  std::set<fs::path> to_compile;
  efsw::FileWatcher fileWatcher;
  EfswFileWatchListener listener( fileWatcher,
                                  std::set<fs::path>{ ".src", ".hsr", ".asp", ".inc", ".em" } );

  auto handle_compile_file = [&]( const fs::path& filepath )
  {
    // Existing file, compile all owners dependent on it.
    if ( auto itr = dependency_owners.find( filepath ); itr != dependency_owners.end() )
    {
      to_compile.insert( itr->second.begin(), itr->second.end() );
    }
    // New file, compile only this file (if it is compilable).
    else
    {
      auto ext = filepath.extension();
      if ( ext.compare( ".src" ) == 0 || ext.compare( ".hsr" ) == 0 || ext.compare( ".asp" ) == 0 )
      {
        to_compile.emplace( filepath );
      }
    }
  };

  auto handle_delete_file = [&]( const fs::path& filepath )
  {
    to_compile.erase( filepath );
    if ( auto itr = owner_dependencies.find( filepath ); itr != owner_dependencies.end() )
    {
      // For all dependencies, remove the deleted file from set of owners for
      // that dependency.
      for ( const auto& depnamepath : itr->second )
      {
        // If dependency exists in map of dependency -> owners
        if ( auto owners_itr = dependency_owners.find( depnamepath );
             owners_itr != dependency_owners.end() )
        {
          // Erase filepath from set of owners
          owners_itr->second.erase( filepath );

          // If owners set is empty, this dependency is no longer used by
          // anything.
          if ( owners_itr->second.empty() )
          {
            //  Erase from map of dependency -> owners.
            dependency_owners.erase( owners_itr );
            listener.remove_file( depnamepath );
            VERBOSE_PRINTLN( "Remove watch file {}", depnamepath );
          }
        }
      }
      owner_dependencies.erase( itr );
    }
  };

  auto add_dir = [&]( const std::string& elem )
  {
    fs::path dir( elem );
    if ( fs::exists( dir ) )
    {
      auto dirpath = fs::canonical( dir );
      if ( listener.add_watch_dir( dirpath ) )
      {
        VERBOSE_PRINTLN( "Add watch dir {}", dirpath );
      }
    }
  };

  for ( const auto& dep_owners : dependency_owners )
  {
    listener.add_file( dep_owners.first );
  }

  for ( const auto& directory : compiled_dirs )
  {
    listener.add_dir( directory );
  }

  for ( const auto& elem : compilercfg.PackageRoot )
  {
    add_dir( elem );
  }

  add_dir( compilercfg.ModuleDirectory );
  add_dir( compilercfg.IncludeDirectory );
  add_dir( compilercfg.PolScriptRoot );

  fileWatcher.watch();

  INFO_PRINTLN( "Entering watch mode. Watching {} files and {} directories. Ctrl-C to stop...",
                dependency_owners.size(), compiled_dirs.size() );

  while ( !Clib::exit_signalled )
  {
    std::this_thread::sleep_for( 1s );
    listener.take_messages( watch_messages );
    if ( !watch_messages.empty() )
    {
      for ( auto const& message : watch_messages )
      {
        const auto& filepath = message.filepath;
        const auto& old_filepath = message.old_filepath;
        VERBOSE_PRINTLN( "Event: filename={}, old_filename={}", filepath, old_filepath );

        // created or modified
        if ( old_filepath.empty() )
        {
          handle_compile_file( filepath );
        }
        // deleted
        else if ( filepath.empty() )
        {
          handle_delete_file( old_filepath );
        }
        // moved
        else
        {
          handle_delete_file( old_filepath );
          handle_compile_file( filepath );
        }
      }
      watch_messages.clear();

      if ( !to_compile.empty() )
      {
        summary.CompiledScripts = 0;
        summary.UpToDateScripts = 0;
        summary.ScriptsWithCompileErrors = 0;
        summary.TotalErrors = 0;
        summary.TotalWarnings = 0;
        progress =
            std::make_unique<ProgressIndicator>( progress_mode && !quiet, to_compile.size() );
        em_parse_tree_cache.clear();
        inc_parse_tree_cache.clear();

        VERBOSE_PRINTLN( "To compile: {}", to_compile );

        Tools::Timer<> timer;
        for ( const auto& filepath : to_compile )
        {
          if ( Clib::exit_signalled )
          {
            break;
          }

          std::set<fs::path> removed_dependencies;
          std::set<fs::path> new_dependencies;
          try
          {
            progress->advance( filepath.generic_string() );
            process_file_wrapper( filepath.generic_string(), &removed_dependencies,
                                  &new_dependencies );
          }
          catch ( ... )
          {
          }

          VERBOSE_PRINTLN( "New dependencies: {} Removed dependencies: {}", new_dependencies,
                           removed_dependencies );

          for ( const auto& depnamepath : removed_dependencies )
          {
            // If a removed dependency has no owner set is empty, we can remove
            // the listener for that dependency.
            if ( dependency_owners.find( depnamepath ) == dependency_owners.end() )
            {
              if ( listener.remove_file( depnamepath ) )
              {
                VERBOSE_PRINTLN( "Remove watch file {}", depnamepath );
              }
            }
          }

          for ( const auto& depnamepath : new_dependencies )
          {
            if ( listener.add_file( depnamepath ) )
            {
              VERBOSE_PRINTLN( "Add watch file {}", depnamepath );
            }
          }
        }

        timer.stop();
        progress->clear();
        if ( compilercfg.DisplaySummary && !quiet )
        {
          DisplaySummary( timer );
        }
        to_compile.clear();
      }
    }
  }
}

/// One group of scripts named by the command line, compiled together.
struct Batch
{
  std::vector<std::string> files;
  bool autocompile = false;  // -A[u]: compile under its own OnlyCompileUpdatedScripts setting
  bool update_only = false;  // what -A[u] asked for, read when the batch was collected
};

/// The scripts of the main script root and of every enabled package.
Batch collect_autocompile()
{
  std::vector<fs::path> dirs;
  compiled_dirs.emplace( fs::canonical( compilercfg.PolScriptRoot ) );

  dirs.emplace_back( compilercfg.PolScriptRoot );
  for ( const auto& pkg : Plib::systemstate.packages )
  {
    compiled_dirs.emplace( fs::canonical( pkg->dir() ) );
    dirs.emplace_back( pkg->dir() );
  }
  return Batch{ collect_files( dirs, false ), true, compilercfg.UpdateOnlyOnAutoCompile };
}

// forspec() takes a plain function pointer, so the sink cannot be captured.
std::vector<std::string>* forspec_sink = nullptr;

/// The files a filespec on the command line names; on Windows it may be a wildcard.
Batch collect_filespec( const char* spec )
{
  Batch batch;
#ifdef _WIN32
  forspec_sink = &batch.files;
  Clib::forspec( spec, []( const char* pathname ) { forspec_sink->emplace_back( pathname ); } );
  forspec_sink = nullptr;
#else
  batch.files.emplace_back( spec );
#endif
  return batch;
}

/**
 * Takes decisions, runs, the compilation, prints summary and cleans after
 */
bool run( int argc, char** argv, int* res )
{
  Clib::enable_exit_signaller();

  load_packages();

  // Determine the run mode and do the compile itself
  Tools::Timer<> timer;
  bool any = false;
  std::vector<Batch> batches;

  for ( int i = 1; i < argc; i++ )
  {
#ifdef __linux__
    if ( argv[i][0] == '-' )
#else
    if ( argv[i][0] == '/' || argv[i][0] == '-' )
#endif
    {
      if ( argv[i][1] == 'A' )
      {
        compilercfg.UpdateOnlyOnAutoCompile = ( argv[i][2] == 'u' );
        any = true;

        batches.push_back( collect_autocompile() );
      }
      else if ( argv[i][1] == 'r' )
      {
        any = true;
        std::string dir( "." );
        bool compile_inc = ( argv[i][2] == 'i' );  // compile .inc files

        ++i;
        if ( i < argc && argv[i] && argv[i][0] != '-' )
          dir.assign( argv[i] );

        compiled_dirs.emplace( fs::canonical( dir ) );
        batches.push_back( Batch{ collect_files( { dir }, compile_inc ), false, false } );
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
      batches.push_back( collect_filespec( argv[i] ) );
    }
  }

  if ( !any && compilercfg.AutoCompileByDefault )
  {
    any = true;
    batches.push_back( collect_autocompile() );
  }

  size_t total = 0;
  for ( const auto& batch : batches )
    total += batch.files.size();
  // -F prints the formatted source to stdout, which a line redrawn over it would corrupt.
  progress = std::make_unique<ProgressIndicator>(
      progress_mode && !quiet && !( format_source && !format_source_inplace ), total );
  // Blanks the line however we leave: without -b the first error throws straight out of here.
  struct ProgressScope
  {
    ~ProgressScope() { progress.reset(); }
  } progress_scope;

  for ( const auto& batch : batches )
  {
    if ( Clib::exit_signalled )
      break;
    // Restored on the way out: without -b the first error throws straight past here.
    struct SettingScope
    {
      bool save = compilercfg.OnlyCompileUpdatedScripts;
      ~SettingScope() { compilercfg.OnlyCompileUpdatedScripts = save; }
    } setting_scope;
    if ( batch.autocompile )
      compilercfg.OnlyCompileUpdatedScripts = batch.update_only;
    process_files( batch.files );
  }

  // Execution is completed: start final/cleanup tasks
  timer.stop();
  progress->clear();

  if ( any && compilercfg.DisplaySummary && !quiet )
  {
    DisplaySummary( timer );
  }

  if ( watch_source )
  {
    EnterWatchMode();
  }

  Plib::systemstate.deinitialize();

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
    ERROR_PRINTLN( "Could not find {}; using defaults.", cfgpath );
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
 * TODO: rework the following cruft from former uotool.cpp
 **********************************************/
#ifdef _WIN32
  Clib::MiniDumper::Initialize();
#endif

  ECompile::read_config_file( s_argc, s_argv );

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 && !compilercfg.AutoCompileByDefault )
  {
    showHelp();
    return 0;  // return "okay"
  }

  watch_source = compilercfg.WatchModeByDefault;
  if ( watch_source )
  {
    compilercfg.GenerateDependencyInfo = true;
    keep_building = true;
    compilercfg.ThreadedCompilation =
        false;  // limitation since dependency gathering is not thread-safe
  }

  int res = ECompile::readargs( s_argc, s_argv );
  if ( res )
  {
    showHelp();
    return res;
  }

  ECompile::apply_configuration();

  if ( !ECompile::quiet )
  {
    // vX.YY
    INFO_PRINTLN( "EScript Compiler v1.{}\n{}\n", ESCRIPT_FILE_VER_CURRENT, POL_COPYRIGHT );
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
}  // namespace Pol::ECompile


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
