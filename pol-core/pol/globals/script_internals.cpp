#include "script_internals.h"

#include "../../clib/stlutil.h"

#include "../uoexec.h"

namespace Pol {
namespace Core {
  ScriptEngineInternalManager scriptEngineInternalManager;

  ScriptEngineInternalManager::ScriptEngineInternalManager() :
	runlist(),
	ranlist(),
	holdlist(),
	notimeoutholdlist(),
	debuggerholdlist(),
	priority_divide(1),
	scrstore(),
	pidlist(),
	next_pid(0)
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
}
}
