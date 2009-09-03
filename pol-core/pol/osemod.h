/*
History
=======
2006/09/17 Shinigami: ::signal_event() will return error on full evene queue

Notes
=======

*/

#ifndef __OSEMOD_H
#define __OSEMOD_H

#include <queue>
#include <map>
#include <time.h>

#ifndef __BSCRIPT_EXECMODL_H
#include "bscript/execmodl.h"
#endif

#include "polclock.h"
#include "polopt.h"

#include "uoexhelp.h"

class OSExecutorModule;
class UOExecutor;

typedef BObjectImp* (OSExecutorModule::*OSExecutorModuleFn)();

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#	undef POSTSTRUCT_PACK
#	define POSTSTRUCT_PACK
#	pragma pack(1)
#endif
struct OSFunctionDef
{
	char funcname[ 33 ];
	OSExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif

typedef std::multimap<polclock_t,UOExecutor*> HoldList;
 
class OSExecutorModule : public ExecutorModule
{
public:
    bool signal_event( BObjectImp* eventimp );
    void suspend();
    void revive();

    explicit OSExecutorModule(Executor& exec);
    ~OSExecutorModule();

    bool critical;
    unsigned char priority;
    bool warn_on_runaway;

    void SleepFor( int secs );
    void SleepForMs( int msecs );

    unsigned long pid() const;
    bool blocked() const;

    bool in_debugger_holdlist() const;
    void revive_debugged();
	BObjectImp* clear_event_queue(); //DAVE

protected:
    bool getCharacterParam( unsigned param, Character*& chrptr );
    BObjectImp* create_debug_context();
    BObjectImp* getpid();
    BObjectImp* getprocess();
	BObjectImp* sleep();
    BObjectImp* sleepms();
    BObjectImp* wait_for_event();
    BObjectImp* events_waiting();
    BObjectImp* set_critical();
	BObjectImp* is_critical();
	BObjectImp* start_script();
	BObjectImp* run_script_to_completion();
	BObjectImp* run_script();
    BObjectImp* mf_parameter();
    BObjectImp* mf_set_debug();
    BObjectImp* mf_Log();
    BObjectImp* mf_system_rpm();
    BObjectImp* mf_set_priority();
    BObjectImp* mf_unload_scripts();
    BObjectImp* mf_set_script_option();
	BObjectImp* mf_set_event_queue_size(); //DAVE 11/24
    BObjectImp* mf_OpenURL();

	BObjectImp* mf_clear_event_queue(); //DAVE
    bool blocked_;
	polclock_t sleep_until_clock_; // 0 if wait forever

    enum { NO_LIST, TIMEOUT_LIST, NOTIMEOUT_LIST, DEBUGGER_LIST } in_hold_list_;
    HoldList::iterator hold_itr_;

    unsigned long pid_;

    enum WAIT_TYPE {
        WAIT_SLEEP,
        WAIT_EVENT
    } wait_type;

    enum { MAX_EVENTQUEUE_SIZE = 20 };
	unsigned short max_eventqueue_size; //DAVE 11/24
    std::queue<BObjectImp*> events_;


	friend class NPCExecutorModule;
	friend void step_scripts(void);
	friend void run_ready(void);
	friend void check_blocked( polclock_t* pclocksleft );
	friend void new_check_blocked(void);
    friend void deschedule_executor( UOExecutor* ex );
    friend polclock_t calc_script_clocksleft( polclock_t now );


    void event_occurred( BObject event );
	// class machinery

protected:
	virtual BObjectImp* execFunc( unsigned idx );
	virtual int functionIndex( const char *func );
	static OSFunctionDef function_table[];
};

inline bool OSExecutorModule::getCharacterParam( unsigned param, Character*& chrptr )
{
    return ::getCharacterParam( exec, param, chrptr );
}

#endif
