/** @file
 *
 * @par History
 */


#ifndef __SCRSCHED_H
#define __SCRSCHED_H

#include <list>
#include <set>

#include "../bscript/eprog.h"
#include "../clib/refptr.h"
#include "polclock.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class EScriptProgram;
}
namespace Module
{
class UOExecutorModule;
}
namespace Core
{
class ScriptDef;
class UOExecutor;

void step_scripts( polclock_t* clocksleft, bool* pactivity );
void start_script( const char* filename, Bscript::BObjectImp* param0,
                   Bscript::BObjectImp* param1 = nullptr );

Module::UOExecutorModule* start_script( const ScriptDef& script, Bscript::BObjectImp* param );
Module::UOExecutorModule* start_script( const ScriptDef& script, Bscript::BObjectImp* param0,
                                        Bscript::BObjectImp* param1,
                                        Bscript::BObjectImp* param2 = nullptr,
                                        Bscript::BObjectImp* param3 = nullptr );
Module::UOExecutorModule* start_script( ref_ptr<Bscript::Program> program,
                                        Bscript::BObjectImp* param );

polclock_t calc_script_clocksleft( polclock_t now );

UOExecutor* create_script_executor();
UOExecutor* create_full_script_executor();

void schedule_executor( UOExecutor* ex );
void deschedule_executor( UOExecutor* ex );

Bscript::BObjectImp* run_executor_to_completion( UOExecutor* ex, const ScriptDef& script );

Bscript::BObjectImp* run_executor_to_completion( UOExecutor& ex, const ScriptDef& script );

bool run_script_to_completion( const char* filename, Bscript::BObjectImp* parameter );
bool run_script_to_completion( const char* filename );

Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script,
                                               Bscript::BObjectImp* param1 );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Bscript::BObjectImp* param1,
                                               Bscript::BObjectImp* param2 );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Bscript::BObjectImp* param1,
                                               Bscript::BObjectImp* param2,
                                               Bscript::BObjectImp* param3 );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Bscript::BObjectImp* param1,
                                               Bscript::BObjectImp* param2,
                                               Bscript::BObjectImp* param3,
                                               Bscript::BObjectImp* param4 );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Bscript::BObjectImp* param1,
                                               Bscript::BObjectImp* param2,
                                               Bscript::BObjectImp* param3,
                                               Bscript::BObjectImp* param4,
                                               Bscript::BObjectImp* param5 );
Bscript::BObjectImp* run_script_to_completion( const ScriptDef& script, Bscript::BObjectImp* param1,
                                               Bscript::BObjectImp* param2,
                                               Bscript::BObjectImp* param3,
                                               Bscript::BObjectImp* param4,
                                               Bscript::BObjectImp* param5,
                                               Bscript::BObjectImp* param6 );

bool call_script( const ScriptDef& script,  // throw()
                  Bscript::BObjectImp* param1 );
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1,
                  Bscript::BObjectImp* param2 );  // throw()
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1, Bscript::BObjectImp* param2,
                  Bscript::BObjectImp* param3 );  // throw()
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1, Bscript::BObjectImp* param2,
                  Bscript::BObjectImp* param3, Bscript::BObjectImp* param4 );
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1, Bscript::BObjectImp* param2,
                  Bscript::BObjectImp* param3, Bscript::BObjectImp* param4,
                  Bscript::BObjectImp* param5 );
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1, Bscript::BObjectImp* param2,
                  Bscript::BObjectImp* param3, Bscript::BObjectImp* param4,
                  Bscript::BObjectImp* param5, Bscript::BObjectImp* param6 );
bool call_script( const ScriptDef& script, Bscript::BObjectImp* param1, Bscript::BObjectImp* param2,
                  Bscript::BObjectImp* param3, Bscript::BObjectImp* param4,
                  Bscript::BObjectImp* param5, Bscript::BObjectImp* param6,
                  Bscript::BObjectImp* param7 );

bool find_uoexec( unsigned int pid, UOExecutor** pp_uoexec );
}
}
#endif
