/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: regen_stats - Tokuno MapDimension doesn't fit blocks of 64x64
 * (WGRID_SIZE)
 * - 2005/09/14 Shinigami: regen_stats - Vital.regen_while_dead implemented
 * - 2006/09/23 Shinigami: wrong data type correction
 * - 2009/11/19 Turley:    lightlevel now supports endless duration - Tomi
 */


#include "tasks.h"

#include <time.h>

#include "../bscript/berror.h"
#include "../bscript/escriptv.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "cmbtcfg.h"
#include "gameclck.h"
#include "globals/script_internals.h"
#include "globals/settings.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "polclock.h"
#include "polsig.h"
#include "profile.h"
#include "realms/realm.h"
#include "uworld.h"
#include "vital.h"

#ifdef _MSC_VER
#pragma warning( \
    disable : 4127 )  // conditional expression is constant (needed because of TICK_PROFILEVAR)
#endif

namespace Pol
{
namespace Core
{
void regen_stats()
{
  THREAD_CHECKPOINT( tasks, 400 );
  time_t now = poltime();
  gameclock_t now_gameclock = read_gameclock();
  THREAD_CHECKPOINT( tasks, 401 );

  unsigned empty_zones = 0;
  unsigned nonempty_zones = 0;

  unsigned wgridx, wgridy;

  auto stat_regen = [&now_gameclock, &now]( Mobile::Character* chr ) {
    THREAD_CHECKPOINT( tasks, 402 );

    if ( chr->has_lightoverride() )
    {
      auto light_until = chr->lightoverride_until();
      if ( light_until < now_gameclock && light_until != ~0u )
      {
        chr->lightoverride( -1 );
        chr->lightoverride_until( 0 );
        THREAD_CHECKPOINT( tasks, 403 );
        chr->check_region_changes();
      }
    }

    if ( chr->has_dblclick_wait() )
    {
      if ( chr->dblclick_wait() < now_gameclock )
        chr->dblclick_wait( 0 );
    }
    if ( chr->has_disable_skills_until() )
    {
      if ( chr->disable_skills_until() < now )
        chr->disable_skills_until( 0 );
    }
    THREAD_CHECKPOINT( tasks, 404 );

    // If in warmode, don't regenerate.
    // If regeneration is currently disabled, don't do it either.
    if ( ( chr->warmode() && settingsManager.combat_config.warmode_inhibits_regen ) ||
         ( now <= chr->disable_regeneration_until ) )
    {
      return;
    }

    THREAD_CHECKPOINT( tasks, 405 );
    for ( const Vital* pVital = FindVital( 0 ); pVital; pVital = pVital->next )
    {
      THREAD_CHECKPOINT( tasks, 406 );
      if ( !chr->dead() || pVital->regen_while_dead )
        chr->regen_vital( pVital );
      THREAD_CHECKPOINT( tasks, 407 );
    }

    if ( !chr->dead() )
    {
      THREAD_CHECKPOINT( tasks, 408 );
      chr->check_undamaged();
      THREAD_CHECKPOINT( tasks, 409 );
    }
  };


  for ( auto& realm : gamestate.Realms )
  {
    wgridx = realm->grid_width();
    wgridy = realm->grid_height();

    for ( unsigned wx = 0; wx < wgridx; ++wx )
    {
      for ( unsigned wy = 0; wy < wgridy; ++wy )
      {
        bool any = false;
        for ( auto& chr : realm->zone[wx][wy].characters )
        {
          any = true;
          stat_regen( chr );
        }
        for ( auto& chr : realm->zone[wx][wy].npcs )
        {
          any = true;
          stat_regen( chr );
        }
        if ( any )
          ++nonempty_zones;
        else
          ++empty_zones;
      }
    }
  }
  THREAD_CHECKPOINT( tasks, 499 );
}

#ifdef _WIN32
static HANDLE m_CurrentProcessHandle;
#endif

void setup_update_rpm( void )
{
#ifdef _WIN32
  m_CurrentProcessHandle = GetCurrentProcess();
  FILETIME d1, d2, k, u;
  GetProcessTimes( m_CurrentProcessHandle, &d1, &d2, &k, &u );
  __int64 kt = *(__int64*)&k;
  __int64 ut = *(__int64*)&u;
  __int64 tot = ( kt + ut ) / 10;  // convert to microseconds
  stateManager.profilevars.last_cputime = 0;
  stateManager.profilevars.last_cpu_total = tot;
#endif
}

void update_rpm( void )
{
  THREAD_CHECKPOINT( tasks, 300 );
  stateManager.profilevars.last_sipm = static_cast<unsigned int>(
      Bscript::escript_instr_cycles - stateManager.profilevars.last_instructions );
  stateManager.profilevars.last_instructions = Bscript::escript_instr_cycles;

  stateManager.profilevars.last_scpm = static_cast<unsigned int>(
      stateManager.profilevars.sleep_cycles - stateManager.profilevars.last_sleep_cycles );
  stateManager.profilevars.last_sleep_cycles = stateManager.profilevars.sleep_cycles;

  stateManager.profilevars.last_script_passes_activity =
      stateManager.profilevars.script_passes_activity;
  stateManager.profilevars.script_passes_activity = 0;
  stateManager.profilevars.last_script_passes_noactivity =
      stateManager.profilevars.script_passes_noactivity;
  stateManager.profilevars.script_passes_noactivity = 0;

  TICK_PROFILEVAR( events );
  TICK_PROFILEVAR( skill_checks );
  TICK_PROFILEVAR( combat_operations );

  TICK_PROFILEVAR( los_checks );
  TICK_PROFILEVAR( polmap_walkheight_calculations );
  TICK_PROFILEVAR( uomap_walkheight_calculations );
  TICK_PROFILEVAR( mobile_movements );


  SET_PROFILEVAR( error_creations, Bscript::BError::creations() );
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
  __int64 kt = *(__int64*)&k;
  __int64 ut = *(__int64*)&u;
  __int64 tot = ( kt + ut ) / 10;  // convert to microseconds
  stateManager.profilevars.last_cputime =
      static_cast<unsigned int>( tot - stateManager.profilevars.last_cpu_total );
  stateManager.profilevars.last_cpu_total = tot;
#endif

  stateManager.profilevars.last_busy_sysload_cycles = stateManager.profilevars.busy_sysload_cycles;
  stateManager.profilevars.last_nonbusy_sysload_cycles =
      stateManager.profilevars.nonbusy_sysload_cycles;
  size_t total_cycles = stateManager.profilevars.busy_sysload_cycles +
                        stateManager.profilevars.nonbusy_sysload_cycles;
  if ( total_cycles )
  {
    stateManager.profilevars.last_sysload =
        stateManager.profilevars.busy_sysload_cycles * 100 / total_cycles;
    stateManager.profilevars.last_sysload_nprocs =
        stateManager.profilevars.sysload_nprocs * 10 / total_cycles;
  }
  // else don't adjust
  stateManager.profilevars.busy_sysload_cycles = 0;
  stateManager.profilevars.nonbusy_sysload_cycles = 0;
  stateManager.profilevars.sysload_nprocs = 0;
  if ( Plib::systemstate.config.watch_sysload )
    INFO_PRINT.Format( "sysload={} ({}) cputime={}\n" )
        << stateManager.profilevars.last_sysload << stateManager.profilevars.last_sysload_nprocs
        << stateManager.profilevars.last_cputime;
  if ( Plib::systemstate.config.log_sysload )
    POLLOG.Format( "sysload={} ({}) cputime={}\n" )
        << stateManager.profilevars.last_sysload << stateManager.profilevars.last_sysload_nprocs
        << stateManager.profilevars.last_cputime;
    // cout << "npc_searches:" << GET_PROFILEVAR_PER_MIN( npc_searches ) << " in " <<
    // GET_PROFILECLOCK_MS( npc_search ) << " ms" << endl;
    // cout << "container_adds:" << GET_PROFILEVAR_PER_MIN( container_adds ) << endl;
    // cout << "container_removes:" << GET_PROFILEVAR_PER_MIN( container_removes ) << endl;

#ifndef NDEBUG
  INFO_PRINT << "activity: " << stateManager.profilevars.last_script_passes_activity
             << "  noactivity: " << stateManager.profilevars.last_script_passes_noactivity << "\n";
#endif
  stateManager.profilevars.last_mapcache_hits = stateManager.profilevars.mapcache_hits;
  stateManager.profilevars.last_mapcache_misses = stateManager.profilevars.mapcache_misses;
  if ( Plib::systemstate.config.watch_mapcache )
    INFO_PRINT << "mapcache: hits=" << stateManager.profilevars.mapcache_hits
               << ", misses=" << stateManager.profilevars.mapcache_misses << ", rate="
               << ( stateManager.profilevars.mapcache_hits
                        ? ( stateManager.profilevars.mapcache_hits * 100 /
                            ( stateManager.profilevars.mapcache_hits +
                              stateManager.profilevars.mapcache_misses ) )
                        : 0 )
               << "%\n";
  stateManager.profilevars.mapcache_hits = 0;
  stateManager.profilevars.mapcache_misses = 0;

  stateManager.profilevars.last_sppm = static_cast<unsigned int>(
      stateManager.profilevars.script_passes - stateManager.profilevars.last_script_passes );
  stateManager.profilevars.last_script_passes = stateManager.profilevars.script_passes;

  TICK_PROFILEVAR( scheduler_passes );
  TICK_PROFILEVAR( noactivity_scheduler_passes );

  if ( Plib::systemstate.config.watch_rpm )
    INFO_PRINT << "scpt: " << stateManager.profilevars.last_sppm
               << "  task: " << ( GET_PROFILEVAR_PER_MIN( scheduler_passes ) ) << "("
               << ( GET_PROFILEVAR_PER_MIN( noactivity_scheduler_passes ) ) << ")"
               << "  scin: " << stateManager.profilevars.last_sipm
               << "  scsl: " << stateManager.profilevars.last_scpm
               << "  MOB: " << get_mobile_count() << "  TLI: " << get_toplevel_item_count() << "\n";

  if ( Plib::systemstate.config.show_realm_info )
  {
    INFO_PRINT << "\nRealm info: \n";
    for ( auto realm : gamestate.Realms )
    {
      INFO_PRINT << "    - " << realm->name() << " (mob: " << realm->mobile_count()
                 << ", off: " << realm->offline_mobile_count()
                 << ", tli: " << realm->toplevel_item_count() << ", mlt: " << realm->multi_count()
                 << ")\n";
    }
  }
  THREAD_CHECKPOINT( tasks, 399 );
}

void update_sysload()
{
  THREAD_CHECKPOINT( tasks, 201 );
  if ( scriptScheduler.getRunlist().empty() )
  {
    ++stateManager.profilevars.nonbusy_sysload_cycles;
  }
  else
  {
    ++stateManager.profilevars.busy_sysload_cycles;
    stateManager.profilevars.sysload_nprocs += scriptScheduler.getRunlist().size();
  }
  THREAD_CHECKPOINT( tasks, 299 );
}

void start_tasks()
{
  setup_update_rpm();
  gamestate.update_rpm_task->start();
  gamestate.regen_stats_task->start();
  gamestate.regen_resources_task->start();
  gamestate.reload_accounts_task->start();
  gamestate.update_sysload_task->start();
  gamestate.reload_pol_cfg_task->start();
}

// script loads
// script starts
// combat operations
// skill checks
}  // namespace Core
}  // namespace Pol
