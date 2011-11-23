/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#ifndef __UOEXEC_H
#define __UOEXEC_H

#ifndef __BSCRIPT_EXECUTOR_H
#	include "../bscript/executor.h"
#endif

#include "../clib/weakptr.h"

class OSExecutorModule;

// const int SCRIPT_RUNAWAY_INTERVAL = 5000;

class UOExecutor : public Executor
{
public:
	OSExecutorModule* os_module;
	UOExecutor();
	virtual ~UOExecutor();

    std::string state();

    u64 instr_cycles;
    u64 sleep_cycles;
    time_t start_time;

    u64 warn_runaway_on_cycle;
    u64 runaway_cycles;

    unsigned int eventmask;
    unsigned short area_size;
    unsigned short speech_size;

    bool can_access_offline_mobiles;
    bool auxsvc_assume_string;
    weak_ptr_owner<UOExecutor> weakptr;

	UOExecutor	*pParent,
				*pChild;
};

#endif
