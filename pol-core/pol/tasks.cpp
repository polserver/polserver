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
#include "../plib/polcfg.h"
#include "../plib/systemstate.h"
#include "cmbtcfg.h"
#include "core.h"
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

  auto stat_regen = [&now_gameclock, &now]( Mobile::Character* chr )
  {
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

    // If in warmode, don't regenerate...
    if ( chr->warmode() )
    {
      switch ( settingsManager.combat_config.warmode_inhibits_regen )
      {
      // at all.
      case WarmodeInhibitsRegenStrategy::Both:
        return;

      // if chr is a player.
      case WarmodeInhibitsRegenStrategy::PlayerOnly:
        if ( !chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
          return;
        break;

      // if chr is an npc.
      case WarmodeInhibitsRegenStrategy::NonplayerOnly:
        if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
          return;
        break;

      // ignore for regeneration purposes.
      case WarmodeInhibitsRegenStrategy::None:
      default:
        break;
      }
    }

    // If regeneration is currently disabled, don't do it.
    if ( now <= chr->disable_regeneration_until )
    {
      return;
    }

    THREAD_CHECKPOINT( tasks, 405 );
    for ( const Vital* pVital : gamestate.vitals )
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
    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& chr : realm->getzone_grid( p ).characters )
      {
        stat_regen( chr );
      }
      for ( auto& chr : realm->getzone_grid( p ).npcs )
      {
        stat_regen( chr );
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
  auto& pvars = stateManager.profilevars;
  pvars.last_instructions_pm =
      Clib::clamp_convert<size_t>( Bscript::escript_instr_cycles - pvars.last_instructions );
  pvars.last_instructions = Bscript::escript_instr_cycles;

  TICK_PROFILEVAR( sleep_cycles );

  pvars.last_script_passes_activity = pvars.script_passes_activity;
  pvars.script_passes_activity = 0;
  pvars.last_script_passes_noactivity = pvars.script_passes_noactivity;
  pvars.script_passes_noactivity = 0;

  TICK_PROFILEVAR( events );
  TICK_PROFILEVAR( skill_checks );
  TICK_PROFILEVAR( combat_operations );

  SET_PROFILEVAR( error_creations, Bscript::BError::creations() );
  TICK_PROFILEVAR( error_creations );

  TICK_PROFILEVAR( tasks_ontime );
  TICK_PROFILEVAR( tasks_late );
  TICK_PROFILEVAR( tasks_late_ticks );

  TICK_PROFILEVAR( scripts_ontime );
  TICK_PROFILEVAR( scripts_late );

  TICK_PROFILEVAR( container_adds );
  TICK_PROFILEVAR( container_removes );

#ifdef _WIN32
  FILETIME d1, d2, k, u;
  GetProcessTimes( m_CurrentProcessHandle, &d1, &d2, &k, &u );
  __int64 kt = *(__int64*)&k;
  __int64 ut = *(__int64*)&u;
  __int64 tot = ( kt + ut ) / 10;  // convert to microseconds
  pvars.last_cputime = static_cast<unsigned int>( tot - pvars.last_cpu_total );
  pvars.last_cpu_total = tot;
#endif

  pvars.last_busy_sysload_cycles = pvars.busy_sysload_cycles;
  pvars.last_nonbusy_sysload_cycles = pvars.nonbusy_sysload_cycles;
  size_t total_cycles = pvars.busy_sysload_cycles + pvars.nonbusy_sysload_cycles;
  if ( total_cycles )
  {
    pvars.last_sysload = pvars.busy_sysload_cycles * 100 / total_cycles;
    pvars.last_sysload_nprocs = pvars.sysload_nprocs * 10 / total_cycles;
  }
  // else don't adjust
  pvars.busy_sysload_cycles = 0;
  pvars.nonbusy_sysload_cycles = 0;
  pvars.sysload_nprocs = 0;
  if ( Plib::systemstate.config.watch_sysload )
    INFO_PRINTLN( "sysload={} ({}) cputime={}", pvars.last_sysload, pvars.last_sysload_nprocs,
                  pvars.last_cputime );
  if ( Plib::systemstate.config.log_sysload )
    POLLOGLN( "sysload={} ({}) cputime={}", pvars.last_sysload, pvars.last_sysload_nprocs,
              pvars.last_cputime );

#ifndef NDEBUG
  INFO_PRINTLN( "activity: {}  noactivity: {}", pvars.last_script_passes_activity,
                pvars.last_script_passes_noactivity );
#endif

  TICK_PROFILEVAR( script_passes );
  TICK_PROFILEVAR( task_passes );
  TICK_PROFILEVAR( noactivity_task_passes );

  if ( Plib::systemstate.config.watch_rpm )
  {
    INFO_PRINTLN(
        "script_passes: {}  task_passes: {}({})  instructions: {}  sleep_cycles: {}  MOB: {}  "
        "TLI: {}",
        GET_PROFILEVAR_PER_MIN( script_passes ), GET_PROFILEVAR_PER_MIN( task_passes ),
        GET_PROFILEVAR_PER_MIN( noactivity_task_passes ), pvars.last_instructions_pm,
        GET_PROFILEVAR_PER_MIN( sleep_cycles ), get_mobile_count(), get_toplevel_item_count() );
    INFO_PRINTLN( "script_passes activity: {} noactivity: {} scriptcount statstic {}",
                  pvars.last_script_passes_activity, pvars.last_script_passes_noactivity,
                  pvars.script_runlist_statistic );
    INFO_PRINTLN( "script_passes duration statistic (us) {}", pvars.script_passes_duration );
    INFO_PRINTLN( "script_passes delay statistic (us) {}", pvars.script_passes_delay );
    pvars.script_passes_delay = Clib::OnlineStatistics{};
    pvars.script_passes_duration = Clib::OnlineStatistics{};
    pvars.script_runlist_statistic = Clib::OnlineStatistics{};
  }
  if ( Plib::systemstate.config.show_realm_info )
  {
    INFO_PRINTLN( "\nRealm info: " );
    for ( auto realm : gamestate.Realms )
    {
      INFO_PRINTLN( "    - {} (mob: {}, off: {}, tli: {}, mlt: {})", realm->name(),
                    realm->mobile_count(), realm->offline_mobile_count(),
                    realm->toplevel_item_count(), realm->multi_count() );
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

void reload_pol_cfg()
{
  THREAD_CHECKPOINT( tasks, 600 );
  try
  {
    struct stat newst;
    stat( "pol.cfg", &newst );

    if ( ( newst.st_mtime != Plib::PolConfig::pol_cfg_stat.st_mtime ) &&
         ( newst.st_mtime < time( nullptr ) - 10 ) )
    {
      POLLOG_INFO( "Reloading pol.cfg..." );
      memcpy( &Plib::PolConfig::pol_cfg_stat, &newst, sizeof Plib::PolConfig::pol_cfg_stat );

      Plib::systemstate.config.read( false );
      apply_polcfg( false );
      POLLOG_INFOLN( "Done!" );
    }
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERRORLN( "Error rereading pol.cfg: {}", ex.what() );
  }
  THREAD_CHECKPOINT( tasks, 699 );
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
