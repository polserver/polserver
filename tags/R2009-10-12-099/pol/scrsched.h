/*
History
=======


Notes
=======

*/

#ifndef __SCRSCHED_H
#define __SCRSCHED_H

#include <list>
#include <set>

#include "polopt.h"

#include "module/osmod.h"

class Executor;
class UOExecutor;
class UOExecutorModule;
class BObjectImp;
class ScriptDef;

typedef std::list<UOExecutor*> ExecList;
extern ExecList runlist;
extern ExecList ranlist;
extern ExecList deadlist;

typedef std::set<UOExecutor*> NoTimeoutHoldList;

extern HoldList holdlist;
extern NoTimeoutHoldList notimeoutholdlist;
extern NoTimeoutHoldList debuggerholdlist;

extern int priority_divide;

void step_scripts( polclock_t* clocksleft, bool* pactivity );
void start_script( const char *filename, 
                   BObjectImp* param0,
                   BObjectImp* param1 = NULL);

UOExecutorModule* start_script( const ScriptDef& script, 
                                 BObjectImp* param ) throw();
UOExecutorModule* start_script( const ScriptDef& script, 
                                 BObjectImp* param0,
                                 BObjectImp* param1,
                                 BObjectImp* param2 = NULL,
                                 BObjectImp* param3 = NULL ) throw();
UOExecutorModule* start_script( ref_ptr<EScriptProgram> program, BObjectImp* param );

polclock_t calc_script_clocksleft( polclock_t now );

UOExecutor *create_script_executor();
UOExecutor *create_full_script_executor();

void schedule_executor( UOExecutor *ex );
void deschedule_executor( UOExecutor *ex );

BObjectImp* run_executor_to_completion( UOExecutor& ex, const ScriptDef& script );

bool run_script_to_completion( const char *filename, BObjectImp* parameter );
bool run_script_to_completion( const char *filename );

BObjectImp* run_script_to_completion( const ScriptDef& script );
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1 );
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1,
                                       BObjectImp* param2 );
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1,
                                       BObjectImp* param2,
                                       BObjectImp* param3 );
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1,
                                       BObjectImp* param2,
                                       BObjectImp* param3,
									   BObjectImp* param4 );
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1,
                                       BObjectImp* param2,
                                       BObjectImp* param3,
									   BObjectImp* param4,
									   BObjectImp* param5);
BObjectImp* run_script_to_completion( const ScriptDef& script, 
                                       BObjectImp* param1,
                                       BObjectImp* param2,
                                       BObjectImp* param3,
									   BObjectImp* param4,
									   BObjectImp* param5,
                                       BObjectImp* param6);

bool call_script( const ScriptDef& script,  // throw()
                  BObjectImp* param1 );
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2 ); // throw()
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2,
                  BObjectImp* param3 ); // throw()
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2,
                  BObjectImp* param3,
				  BObjectImp* param4 );
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2,
                  BObjectImp* param3,
				  BObjectImp* param4,
				  BObjectImp* param5);
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2,
                  BObjectImp* param3,
				  BObjectImp* param4,
				  BObjectImp* param5,
                  BObjectImp* param6);
bool call_script( const ScriptDef& script, 
                  BObjectImp* param1,
                  BObjectImp* param2,
                  BObjectImp* param3,
				  BObjectImp* param4,
				  BObjectImp* param5,
                  BObjectImp* param6,
                  BObjectImp* param7);
void cleanup_scripts();
bool find_uoexec( unsigned long pid, UOExecutor** pp_uoexec );
#endif
