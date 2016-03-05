#include "script_internals.h"

#include "../../clib/stlutil.h"

#include "../uoexec.h"

namespace Pol
{
namespace Core
{
ScriptEngineInternalManager scriptEngineInternalManager;

// This number is intended so that PID and custom GUMPIDS will never clash together
// and to avoid breaking the old assumption that gumpid == pid when gumpid has been
// automatically generated (for backward compatibility).
// Custom gumpids must always be < PID_MIN.
const unsigned int ScriptEngineInternalManager::PID_MIN = 0x01000000;

ScriptEngineInternalManager::ScriptEngineInternalManager() :
  runlist(),
  ranlist(),
  holdlist(),
  notimeoutholdlist(),
  debuggerholdlist(),
  priority_divide(1),
  scrstore(),
  pidlist(),
  next_pid(PID_MIN)
{

}

ScriptEngineInternalManager::~ScriptEngineInternalManager()
{
}

// Note, when the program exits, each executor in these queues
// will be deleted by cleanup_scripts()
// Therefore, any object that owns an executor must be destroyed
// before cleanup_scripts() is called.
void ScriptEngineInternalManager::deinitialize()
{
  scrstore.clear();
  Clib::delete_all( runlist );
  while ( !holdlist.empty() )
  {
    delete ( ( *holdlist.begin() ).second );
    holdlist.erase( holdlist.begin() );
  }
  while ( !notimeoutholdlist.empty() )
  {
    delete ( *notimeoutholdlist.begin() );
    notimeoutholdlist.erase( notimeoutholdlist.begin() );
  }
  while ( !debuggerholdlist.empty() )
  {
    delete ( *debuggerholdlist.begin() );
    debuggerholdlist.erase( debuggerholdlist.begin() );
  }
}

ScriptEngineInternalManager::Memory ScriptEngineInternalManager::estimateSize() const
{
  Memory usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.script_size = sizeof(int) /*priority_divide*/
                      + sizeof (unsigned int) /*next_pid*/
                      + ( sizeof(unsigned int)+sizeof( UOExecutor*) + ( sizeof(void*) * 3 + 1 ) / 2 ) * pidlist.size();

  for ( const auto& script : scrstore )
  {
    usage.scriptstorage_size += ( sizeof(void*)* 3 + 1 ) / 2;
    usage.scriptstorage_size += script.first.capacity();
    if (script.second.get() != nullptr)
      usage.scriptstorage_size += script.second->sizeEstimate();
  }
  usage.scriptstorage_count = scrstore.size();


  usage.script_size += 3 * sizeof(UOExecutor**)+runlist.size() * sizeof( UOExecutor*);
  for ( const auto& exec : runlist )
  {
    if (exec != nullptr)
      usage.script_size += exec->sizeEstimate();
  }
  usage.script_count += runlist.size();

  usage.script_size += 3 * sizeof(UOExecutor**)+ranlist.size() * sizeof( UOExecutor*);
  for ( const auto& exec : ranlist )
  {
    if (exec != nullptr)
      usage.script_size += exec->sizeEstimate();
  }
  usage.script_count += ranlist.size();

  for ( const auto& hold : holdlist )
  {
    if (hold.second != nullptr)
      usage.script_size += hold.second->sizeEstimate();
    usage.script_size += sizeof( Core::polclock_t ) + ( sizeof(void*)* 3 + 1 ) / 2;
  }
  usage.script_count += holdlist.size();

  usage.script_size += 3 * sizeof( void*);
  for ( const auto& hold : notimeoutholdlist )
  {
    if (hold != nullptr)
      usage.script_size += hold->sizeEstimate( ) + 3 * sizeof( void*);
  }
  usage.script_count += notimeoutholdlist.size();

  usage.script_size += 3 * sizeof( void*);
  for ( const auto& hold : debuggerholdlist )
  {
    if (hold != nullptr)
      usage.script_size += hold->sizeEstimate( ) + 3 * sizeof( void*);
  }
  usage.script_count += debuggerholdlist.size();


  return usage;
}

}
}
