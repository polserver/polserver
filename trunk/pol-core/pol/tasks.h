/*
History
=======


Notes
=======

*/
#ifndef __TASK_H
#define __TASK_H
#include "schedule.h"
namespace Pol {
  namespace Core {
	extern PeriodicTask update_rpm_task;
	extern PeriodicTask regen_stats_task;
	extern PeriodicTask regen_resources_task;
	extern PeriodicTask reload_accounts_task;
	extern PeriodicTask write_account_task;
	extern PeriodicTask update_sysload_task;
	extern PeriodicTask reload_pol_cfg_task;
  }
}
#endif
