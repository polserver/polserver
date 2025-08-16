/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#include "scrstore.h"

#include "../bscript/eprog.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "globals/script_internals.h"
#include "globals/state.h"
#include "profile.h"
#include "scrdef.h"

#include <iterator>


namespace Pol
{
namespace Items
{
void preload_test_scripts();
void preload_test_scripts( const std::string& );
}  // namespace Items
namespace Core
{
bool script_loaded( ScriptDef& sd )
{
  ScriptStorage::iterator itr = scriptScheduler.scrstore.find( sd.name().c_str() );
  return ( itr != scriptScheduler.scrstore.end() );
}

ref_ptr<Bscript::EScriptProgram> find_script( const std::string& name, bool complain_if_not_found,
                                              bool cache_script )
{
  ScriptStorage::iterator itr = scriptScheduler.scrstore.find( name.c_str() );
  if ( itr != scriptScheduler.scrstore.end() )
  {
    if ( cache_script )
    {
      return ( *itr ).second;
    }
    else
    {
      ref_ptr<Bscript::EScriptProgram> res( ( *itr ).second );
      scriptScheduler.scrstore.erase( itr );
      return res;
    }
  }

  ref_ptr<Bscript::EScriptProgram> program( new Bscript::EScriptProgram );
  std::string pathname = "scripts/";
  pathname += name.c_str();
  if ( name.find( ".ecl" ) == std::string::npos )
    pathname += ".ecl";

  if ( program->read( pathname.c_str() ) != 0 )
  {
    if ( complain_if_not_found )
    {
      POLLOG_ERRORLN( "Unable to read script '{}'", pathname );
    }
    return ref_ptr<Bscript::EScriptProgram>( nullptr );
  }

  if ( cache_script )
  {
    std::string tmpname = name;
    Clib::mklowerASCII( tmpname );
    scriptScheduler.scrstore.insert( ScriptStorage::value_type( tmpname.c_str(), program ) );
  }

  return program;
}

// NOTE,we assume this has directory info (including scripts/ or pkg/xx)
//      as well as ".ecl" on the end.
ref_ptr<Bscript::EScriptProgram> find_script2( const ScriptDef& script, bool complain_if_not_found,
                                               bool cache_script )
{
  ScriptStorage::iterator itr = scriptScheduler.scrstore.find( script.c_str() );
  if ( itr != scriptScheduler.scrstore.end() )
    return ( *itr ).second;

  ref_ptr<Bscript::EScriptProgram> program( new Bscript::EScriptProgram );
  program->pkg = script.pkg();

  if ( program->read( script.c_str() ) != 0 )
  {
    if ( complain_if_not_found )
    {
      POLLOG_ERRORLN( "Unable to read script '{}'", script.name() );
    }
    return ref_ptr<Bscript::EScriptProgram>( nullptr );
  }

  if ( cache_script )
  {
    std::string tmpname = script.name();
    Clib::mklowerASCII( tmpname );
    scriptScheduler.scrstore.insert( ScriptStorage::value_type( tmpname.c_str(), program ) );
  }

  return program;
}


int unload_script( const std::string& name_in )
{
  int n = 0;
  ScriptStorage::iterator itr = scriptScheduler.scrstore.begin();
  while ( itr != scriptScheduler.scrstore.end() )
  {
    ScriptStorage::iterator cur = itr;
    ++itr;

    if ( strstr( cur->first.c_str(), name_in.c_str() ) )
    {
      std::string nm = cur->first;
      const char* nm_cstr = nm.c_str();
      INFO_PRINTLN( "Unloading {}", nm_cstr );
      scriptScheduler.scrstore.erase( cur );
      ++n;

      // dave added 1/30/3, FIXME: if in the future we have to add any other scripts to
      // auto-reload, put the names in a data structure :P
      if ( strstr( nm_cstr, "unequiptest.ecl" ) )
      {
        Items::preload_test_scripts( std::string( "unequiptest.ecl" ) );
        continue;
      }

      if ( strstr( nm_cstr, "equiptest.ecl" ) )
      {
        Items::preload_test_scripts( std::string( "equiptest.ecl" ) );
        continue;
      }
    }
  }
  return n;
}

int unload_all_scripts()
{
  int n = static_cast<int>( scriptScheduler.scrstore.size() );
  scriptScheduler.scrstore.clear();
  Items::preload_test_scripts();  // dave added 1/30/3, no other time do we reload unequiptest and
                                  // equiptest
  return n;
}

void log_all_script_cycle_counts( bool clear_counters )
{
  u64 total_instr = 0;
  for ( const auto& scr : scriptScheduler.scrstore )
  {
    total_instr += scr.second->instr_cycles;
  }

  POLLOGLN(
      "Task passes: {}\n"
      "Script passes:    {}",
      ( GET_PROFILEVAR( task_passes ) ), GET_PROFILEVAR( script_passes ) );

  std::string tmp = fmt::format( "{:<38} {:>12} {:>6} {:>12} {:>6}\n", "Script", "cycles", "incov",
                                 "cyc/invoc", "%" );
  for ( const auto& scr : scriptScheduler.scrstore )
  {
    Bscript::EScriptProgram* eprog = scr.second.get();
    double cycle_percent =
        total_instr != 0 ? ( static_cast<double>( eprog->instr_cycles ) / total_instr * 100.0 ) : 0;
    fmt::format_to( std::back_inserter( tmp ), "{:<38} {:>12} {:>6} {:>12} {:>6}\n", eprog->name,
                    eprog->instr_cycles, eprog->invocations,
                    eprog->instr_cycles / ( eprog->invocations ? eprog->invocations : 1 ),
                    cycle_percent );
    if ( clear_counters )
    {
      eprog->instr_cycles = 0;
      eprog->invocations = eprog->count() - 1;  // 1 count is the scrstore's
    }
  }
  POLLOG( tmp );
  if ( clear_counters )
    POLLOGLN( "Profiling counters cleared." );
}


void clear_script_profile_counters()
{
  for ( const auto& scr : scriptScheduler.scrstore )
  {
    Bscript::EScriptProgram* eprog = scr.second.get();
    eprog->instr_cycles = 0;
    eprog->invocations = eprog->count() - 1;  // 1 count is the scrstore's
  }

  POLLOGLN( "Profiling counters cleared." );
}
}  // namespace Core
}  // namespace Pol
