/*
History
=======
2005/01/23 Shinigami: regen_stats - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2005/09/14 Shinigami: regen_stats - Vital.regen_while_dead implemented
2006/09/23 Shinigami: wrong data type correction
2009/11/19 Turley:    lightlevel now supports endless duration - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../bscript/berror.h"
#include "../bscript/escriptv.h"

#include "../clib/endian.h"
#include "../clib/logfile.h"
#include "../plib/realm.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "cmbtcfg.h"
#include "decay.h"
#include "polcfg.h"
#include "polclock.h"
#include "polfile.h"
#include "polsig.h"
#include "profile.h"
#include "realms.h"
#include "schedule.h"
#include "scrsched.h"
#include "sockio.h"
#include "statmsg.h"
#include "tasks.h"
#include "ufunc.h"
#include "uvars.h"
#include "uworld.h"

void regen_stats(void)
{
    // cout << "regen!";
    THREAD_CHECKPOINT( tasks, 400 );
	time_t now = poltime();
    gameclock_t now_gameclock = read_gameclock();
    THREAD_CHECKPOINT( tasks, 401 );

    unsigned empty_zones = 0;
    unsigned nonempty_zones = 0;

	unsigned wgridx, wgridy;
	vector<Realm*>::iterator itr;
	for( itr = Realms->begin(); itr != Realms->end(); ++itr )
	{
		Realm* realm = *itr;
		wgridx = realm->width() / WGRID_SIZE;
		wgridy = realm->height() / WGRID_SIZE;
    
    // Tokuno-Fix
    if (wgridx * WGRID_SIZE < realm->width())
      wgridx++;
    if (wgridy * WGRID_SIZE < realm->height())
      wgridy++;
    
    for( unsigned wx = 0; wx < wgridx; ++wx )
		{
			for( unsigned wy = 0; wy < wgridy; ++wy )
			{
				bool any = false;
				ZoneCharacters& wchr = realm->zone[wx][wy].characters;
				ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end();
				for( ; itr != end; ++itr )
				{
					Character* chr = *itr;
					any = true;
					THREAD_CHECKPOINT( tasks, 402 );

					if (chr->lightoverride != -1)
					{
						if (chr->lightoverride_until < now_gameclock && chr->lightoverride_until != ~0Lu)
						{
							chr->lightoverride = -1;
							chr->lightoverride_until = 0;
							THREAD_CHECKPOINT( tasks, 403 );
							chr->check_region_changes();
						}
					}
					THREAD_CHECKPOINT( tasks, 404 );

					// If in warmode, don't regenerate.
					// If regeneration is currently disabled, don't do it either.
					if ((chr->warmode && combat_config.warmode_inhibits_regen) || 
						(now <= chr->disable_regeneration_until) )
					{
						continue;
					}

					THREAD_CHECKPOINT( tasks, 405 );
					for( const Vital* pVital = FindVital(0); pVital; pVital = pVital->next )
					{
						THREAD_CHECKPOINT( tasks, 406 );
						if (!chr->dead() || pVital->regen_while_dead)
							chr->regen_vital(pVital);
						THREAD_CHECKPOINT( tasks, 407 );
					}

					if (!chr->dead())
					{
						THREAD_CHECKPOINT( tasks, 408 );
						chr->check_undamaged();
						THREAD_CHECKPOINT( tasks, 409 );
					}
				}
				if (any)
					++nonempty_zones;
				else
					++empty_zones;
			}
		}
	}
    THREAD_CHECKPOINT( tasks, 499 );
}
PeriodicTask regen_stats_task( regen_stats, 5, "Regen" );

#ifdef _WIN32
static HANDLE m_CurrentProcessHandle;
#endif

void setup_update_rpm( void )
{
#ifdef _WIN32
    m_CurrentProcessHandle = GetCurrentProcess();
    FILETIME d1, d2, k, u;
    GetProcessTimes( m_CurrentProcessHandle, &d1, &d2, &k, &u );
    __int64 kt = * (__int64*) &k;
    __int64 ut = * (__int64*) &u;
    __int64 tot = (kt+ut)/10; // convert to microseconds
    last_cputime = 0;
    last_cpu_total = tot;
#endif
}

void update_rpm( void )
{
    THREAD_CHECKPOINT( tasks, 300 );
    last_sipm = static_cast<unsigned long>(escript_instr_cycles - last_instructions);
    last_instructions = escript_instr_cycles;

    last_scpm = static_cast<unsigned long>(sleep_cycles - last_sleep_cycles);
    last_sleep_cycles = sleep_cycles;

    last_script_passes_activity = script_passes_activity;
    script_passes_activity = 0;
    last_script_passes_noactivity = script_passes_noactivity;
    script_passes_noactivity = 0;

    TICK_PROFILEVAR( events );
    TICK_PROFILEVAR( skill_checks );
    TICK_PROFILEVAR( combat_operations );

    TICK_PROFILEVAR( los_checks );
    TICK_PROFILEVAR( polmap_walkheight_calculations );
    TICK_PROFILEVAR( uomap_walkheight_calculations );
    TICK_PROFILEVAR( mobile_movements );


    SET_PROFILEVAR( error_creations, BError::creations() );
    TICK_PROFILEVAR( error_creations );

    TICK_PROFILEVAR( tasks_ontime );
    TICK_PROFILEVAR( tasks_late );
    TICK_PROFILEVAR( tasks_late_ticks );

    TICK_PROFILEVAR( scripts_ontime );
    TICK_PROFILEVAR( scripts_late );

    TICK_PROFILEVAR( npc_searches );
    ROLL_PROFILECLOCK( npc_search );

    TICK_PROFILEVAR( container_adds );
    TICK_PROFILEVAR( container_removes );

#ifdef _WIN32
    FILETIME d1, d2, k, u;
    GetProcessTimes( m_CurrentProcessHandle, &d1, &d2, &k, &u );
    __int64 kt = * (__int64*) &k;
    __int64 ut = * (__int64*) &u;
    __int64 tot = (kt+ut)/10; // convert to microseconds
    last_cputime = static_cast<unsigned long>(tot-last_cpu_total);
    last_cpu_total = tot;
#endif

    last_busy_sysload_cycles = busy_sysload_cycles;
    last_nonbusy_sysload_cycles = nonbusy_sysload_cycles;
    unsigned long total_cycles = busy_sysload_cycles + nonbusy_sysload_cycles;
    if (total_cycles)
    {
        last_sysload = busy_sysload_cycles * 100 / total_cycles;
        last_sysload_nprocs = sysload_nprocs * 10 / total_cycles;
    }
    // else don't adjust
    busy_sysload_cycles = 0;
    nonbusy_sysload_cycles = 0;
    sysload_nprocs = 0;
    if (config.watch_sysload)
        cout << "sysload=" << last_sysload << " (" << last_sysload_nprocs << ")" 
             << " cputime=" << last_cputime << endl;
    if (config.log_sysload)
        Log( "sysload=%lu (%lu) cputime=%lu\n", last_sysload, last_sysload_nprocs, last_cputime );
    //cout << "npc_searches:" << GET_PROFILEVAR_PER_MIN( npc_searches ) << " in " << GET_PROFILECLOCK_MS( npc_search ) << " ms" << endl;
    //cout << "container_adds:" << GET_PROFILEVAR_PER_MIN( container_adds ) << endl;
    //cout << "container_removes:" << GET_PROFILEVAR_PER_MIN( container_removes ) << endl;

#ifndef NDEBUG
    cout << "activity: " << last_script_passes_activity
         << "  noactivity: " << last_script_passes_noactivity
         << endl;
#endif
    last_mapcache_hits = mapcache_hits;
    last_mapcache_misses = mapcache_misses;
    if (config.watch_mapcache)
        cout << "mapcache: hits=" << mapcache_hits << ", misses=" << mapcache_misses
             << ", rate=" << (mapcache_hits ? (mapcache_hits * 100 / (mapcache_hits+mapcache_misses) ) : 0)<< "%"
             << endl;
    mapcache_hits = 0;
    mapcache_misses = 0;

    if (config.multithread)
    {
        last_sppm = static_cast<unsigned long>(script_passes - last_script_passes);
        last_script_passes = script_passes;

        TICK_PROFILEVAR( scheduler_passes );
        TICK_PROFILEVAR( noactivity_scheduler_passes );

        if (config.watch_rpm)
            cout << "scpt: " << last_sppm 
                 << "  task: " << GET_PROFILEVAR_PER_MIN( scheduler_passes );
            if (GET_PROFILEVAR_PER_MIN(noactivity_scheduler_passes))
                cout << "(" << GET_PROFILEVAR_PER_MIN(noactivity_scheduler_passes) << ")";
            cout << "  scin: " << last_sipm 
                 << "  scsl: " << last_scpm
                 << "  MOB: " << get_mobile_count()
                 << "  TLI: " << get_toplevel_item_count()
                 << endl;
    }
    else
    {
        last_rpm = rotations - last_rotations;
        last_rotations = rotations;
    
		//fixme realms
        cycles_per_decay_worldzone = last_rpm / (WGRID_X * WGRID_Y / 10);
        if (cycles_per_decay_worldzone < 1)
            cycles_per_decay_worldzone = 1;
        cycles_until_decay_worldzone = cycles_per_decay_worldzone;

        if (config.watch_rpm) 
            cout << "RPM: " << last_rpm 
                 << "   SIPM: " << last_sipm 
                 << "   SCPM: " << last_scpm
                 << "   SI/R: " << (last_rpm?(last_sipm / last_rpm):0) 
                 << "   SC/R: " << (last_rpm?(last_scpm / last_rpm):0) 
                 << endl;
    }
    THREAD_CHECKPOINT( tasks, 399 );
}
PeriodicTask update_rpm_task( update_rpm, 60, "RPM" );

void update_sysload(void)
{
    THREAD_CHECKPOINT( tasks, 201 );
    if (runlist.empty())
    {
        ++nonbusy_sysload_cycles;
    }
    else
    {
        ++busy_sysload_cycles;
        sysload_nprocs += runlist.size();
    }
    THREAD_CHECKPOINT( tasks, 299 );
}
PeriodicTask update_sysload_task( update_sysload, 1, "SYSLOAD" );

void start_tasks()
{
    setup_update_rpm();
    update_rpm_task.start();
    regen_stats_task.start();
    regen_resources_task.start();
    reload_accounts_task.start();
    update_sysload_task.start();
    reload_pol_cfg_task.start();
}

// script loads
// script starts
// combat operations
// skill checks
