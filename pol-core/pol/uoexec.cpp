#include "uoexec.h"

#include "polclock.h"
#include "polcfg.h"

#include "module/osmod.h"
#include "watch.h"

#include "../clib/logfacility.h"

namespace Pol {
    namespace Core {
        UOExecutor::UOExecutor() :
            Executor(),
            os_module(NULL),
            instr_cycles(0),
            sleep_cycles(0),
            start_time(poltime()),
            warn_runaway_on_cycle(config.runaway_script_threshold),
            runaway_cycles(0),
            eventmask(0),
            area_size(0),
            speech_size(1),
            can_access_offline_mobiles(false),
            auxsvc_assume_string(false),
            pParent(NULL),
            pChild(NULL)
        {
            weakptr.set(this);
            os_module = new Module::OSExecutorModule(*this);
            addModule(os_module);
        }

        UOExecutor::~UOExecutor()
        {
            // note, the os_module isn't deleted here because
            // the Executor deletes its ExecutorModules.
            if ((instr_cycles >= 500) && watch.profile_scripts)
            {
                int elapsed = static_cast<int>(poltime() - start_time); // Doh! A script can't run more than 68 years, for this to work.
                POLLOG_ERROR.Format("Script {}: {} instr cycles, {} sleep cycles, {} seconds\n")
                    << scriptname() << instr_cycles << sleep_cycles << elapsed;
            }

            pParent = NULL;
            pChild = NULL;
        }

        std::string UOExecutor::state()
        {
            if (halt())
                return "Debugging";
            else if (os_module->blocked())
                return "Sleeping";
            else
                return "Running";
        }

        size_t UOExecutor::sizeEstimate() const
        {
            return sizeof(UOExecutor) + base::sizeEstimate();
        }
    }
}