/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 *                         ClearTextPasswords now default is False.
 * - 2009/10/14 Turley:    added bool LogfileTimestampEveryLine
 * - 2009/12/02 Turley:    added MaxTileID -Tomi
 * - 2009/12/04 Turley:    cleanup "MasterKey1","MasterKey2","ClientVersion","KeyFile" - Tomi
 * - 2010/02/04 Turley:    polcfg.discard_old_events discards oldest event if queue is full
 */


#include "polcfg.h"

#include <cstdio>
#include <exception>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <time.h>

#include "../bscript/config.h"
#include "../clib/Debugging/ExceptionParser.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/mdump.h"
#include "../clib/passert.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "../plib/uoinstallfinder.h"
// TODO: get rid of the dependencies and move to plib
#include "core.h"           // todo save_full does not belong here
#include "globals/state.h"  // todo polsig dependency
#include "globals/uvars.h"  // todo split write task
#include "objtype.h"
#include "polsig.h"    // thread_checkpoint
#include "proplist.h"  // todo like uvars

namespace Pol
{
namespace Core
{
struct stat PolConfig::pol_cfg_stat;

void PolConfig::read_pol_config( bool initial_load )
{
  Clib::ConfigFile cf( "pol.cfg" );
  Clib::ConfigElem elem;

  cf.readraw( elem );

  if ( initial_load )
  {
    stat( "pol.cfg", &pol_cfg_stat );

    // these config options can't change after startup.
    Plib::systemstate.config.uo_datafile_root = Plib::UOInstallFinder::remove_elem( elem );
    Plib::systemstate.config.uo_datafile_root =
        Clib::normalized_dir_form( Plib::systemstate.config.uo_datafile_root );

    Plib::systemstate.config.world_data_path = elem.remove_string( "WorldDataPath", "data/" );
    Plib::systemstate.config.world_data_path =
        Clib::normalized_dir_form( Plib::systemstate.config.world_data_path );

    Plib::systemstate.config.realm_data_path = elem.remove_string( "RealmDataPath", "realm/" );
    Plib::systemstate.config.realm_data_path =
        Clib::normalized_dir_form( Plib::systemstate.config.realm_data_path );

    Plib::systemstate.config.pidfile_path = elem.remove_string( "PidFilePath", "./" );
    Plib::systemstate.config.pidfile_path =
        Clib::normalized_dir_form( Plib::systemstate.config.pidfile_path );

    Plib::systemstate.config.check_integrity = true;  // elem.remove_bool( "CheckIntegrity", true );
    Plib::systemstate.config.count_resource_tiles = elem.remove_bool( "CountResourceTiles", false );
    Plib::systemstate.config.web_server = elem.remove_bool( "WebServer", false );
    Plib::systemstate.config.web_server_port = elem.remove_ushort( "WebServerPort", 8080 );

    unsigned short max_tile = elem.remove_ushort( "MaxTileID", 0 );
    if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX &&
         max_tile != 0 )
    {
      max_tile = UOBJ_DEFAULT_MAX;
    }
    Plib::systemstate.config.max_tile_id = max_tile;

    unsigned int max_obj = elem.remove_unsigned( "MaxObjtype", EXTOBJ_HIGHEST_DEFAULT );
    if ( max_obj < EXTOBJ_HIGHEST_DEFAULT || max_obj > 0xFFFFFFFF )
      Plib::systemstate.config.max_objtype = EXTOBJ_HIGHEST_DEFAULT;
    else
      Plib::systemstate.config.max_objtype = max_obj;

    Plib::systemstate.config.ignore_load_errors = elem.remove_bool( "IgnoreLoadErrors", false );

    Plib::systemstate.config.debug_port = elem.remove_ushort( "DebugPort", 0 );

    Plib::systemstate.config.account_save = elem.remove_int( "AccountDataSave", -1 );
    if ( Plib::systemstate.config.account_save > 0 )
    {
      gamestate.write_account_task->set_secs( Plib::systemstate.config.account_save );
      gamestate.write_account_task->start();
    }
  }
  Plib::systemstate.config.verbose = elem.remove_bool( "Verbose", false );
  Plib::systemstate.config.watch_mapcache = elem.remove_bool( "WatchMapCache", false );
  Plib::systemstate.config.loglevel = elem.remove_ushort( "LogLevel", 0 );
  Plib::systemstate.config.select_timeout_usecs = elem.remove_ushort( "SelectTimeout", 10 );
  Plib::systemstate.config.loginserver_timeout_mins =
      elem.remove_ushort( "LoginServerTimeout", 10 );
  Plib::systemstate.config.watch_rpm = elem.remove_bool( "WatchRpm", false );
  Plib::systemstate.config.watch_sysload = elem.remove_bool( "WatchSysLoad", false );
  Plib::systemstate.config.log_sysload = elem.remove_bool( "LogSysLoad", false );
  Plib::systemstate.config.inhibit_saves = elem.remove_bool( "InhibitSaves", false );
  Plib::systemstate.config.log_script_cycles = elem.remove_bool( "LogScriptCycles", false );
  Plib::systemstate.config.web_server_local_only = elem.remove_bool( "WebServerLocalOnly", true );
  Plib::systemstate.config.web_server_debug = elem.remove_ushort( "WebServerDebug", 0 );
  Plib::systemstate.config.web_server_password = elem.remove_string( "WebServerPassword", "" );

  Plib::systemstate.config.profile_cprops = elem.remove_bool( "ProfileCProps", false );

  Plib::systemstate.config.cache_interactive_scripts =
      elem.remove_bool( "CacheInteractiveScripts", true );
  Plib::systemstate.config.show_speech_colors = elem.remove_bool( "ShowSpeechColors", false );
  Plib::systemstate.config.require_spellbooks = elem.remove_bool( "RequireSpellbooks", true );

  Plib::systemstate.config.enable_secure_trading = elem.remove_bool( "EnableSecureTrading", false );
  Plib::systemstate.config.runaway_script_threshold =
      elem.remove_ulong( "RunawayScriptThreshold", 5000 );

  Plib::systemstate.config.min_cmdlvl_ignore_inactivity =
      elem.remove_ushort( "MinCmdLvlToIgnoreInactivity", 1 );
  Plib::systemstate.config.inactivity_warning_timeout =
      elem.remove_ushort( "InactivityWarningTimeout", 4 );
  Plib::systemstate.config.inactivity_disconnect_timeout =
      elem.remove_ushort( "InactivityDisconnectTimeout", 5 );

  Plib::systemstate.config.min_cmdlevel_to_login = elem.remove_ushort( "MinCmdlevelToLogin", 0 );

  Bscript::escript_config.max_call_depth = elem.remove_ulong( "MaxCallDepth", 100 );
  Clib::passert_dump_stack = elem.remove_bool( "DumpStackOnAssertionFailure", false );

  std::string tmp = elem.remove_string( "AssertionFailureAction", "abort" );
  if ( Clib::strlowerASCII( tmp ) == "abort" )
  {
    Clib::passert_shutdown = false;
    Clib::passert_nosave = false;
    Clib::passert_abort = true;
    Plib::systemstate.config.assertion_shutdown_save_type =
        SAVE_FULL;  // should never come into play
  }
  else if ( Clib::strlowerASCII( tmp ) == "continue" )
  {
    Clib::passert_shutdown = false;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
    Plib::systemstate.config.assertion_shutdown_save_type =
        SAVE_FULL;  // should never come into play
  }
  else if ( Clib::strlowerASCII( tmp ) == "shutdown" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
    Plib::systemstate.config.assertion_shutdown_save_type = SAVE_FULL;
  }
  else if ( Clib::strlowerASCII( tmp ) == "shutdown-nosave" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = true;
    Clib::passert_abort = false;
    Plib::systemstate.config.assertion_shutdown_save_type =
        SAVE_FULL;  // should never come into play
  }
  else if ( Clib::strlowerASCII( tmp ) == "shutdown-save-full" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
    Plib::systemstate.config.assertion_shutdown_save_type = SAVE_FULL;
  }
  else if ( Clib::strlowerASCII( tmp ) == "shutdown-save-incremental" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
    Plib::systemstate.config.assertion_shutdown_save_type = SAVE_INCREMENTAL;
  }
  else
  {
    Clib::passert_shutdown = false;
    Clib::passert_abort = true;
    Plib::systemstate.config.assertion_shutdown_save_type =
        SAVE_FULL;  // should never come into play
    POLLOG_ERROR.Format(
        "Unknown pol.cfg AssertionFailureAction value: {} (expected abort, continue, shutdown, or "
        "shutdown-nosave)\n" )
        << tmp;
  }

  tmp = elem.remove_string( "ShutdownSaveType", "full" );
  if ( Clib::strlowerASCII( tmp ) == "full" )
  {
    Plib::systemstate.config.shutdown_save_type = SAVE_FULL;
  }
  else if ( Clib::strlowerASCII( tmp ) == "incremental" )
  {
    Plib::systemstate.config.shutdown_save_type = SAVE_INCREMENTAL;
  }
  else
  {
    Plib::systemstate.config.shutdown_save_type = SAVE_FULL;
    POLLOG_ERROR.Format(
        "Unknown pol.cfg ShutdownSaveType value: {} (expected full or incremental)\n" )
        << tmp;
  }

  Plib::systemstate.config.display_unknown_packets =
      elem.remove_bool( "DisplayUnknownPackets", false );
  Plib::systemstate.config.exp_los_checks_map = elem.remove_bool( "ExpLosChecksMap", true );
  Plib::systemstate.config.enable_debug_log = elem.remove_bool( "EnableDebugLog", false );
  Plib::systemstate.config.debug_password = elem.remove_string( "DebugPassword", "" );
  Plib::systemstate.config.debug_local_only = elem.remove_bool( "DebugLocalOnly", true );

  Plib::systemstate.config.log_traces_when_stuck =
      elem.remove_bool( "ThreadStacktracesWhenStuck", false );

  Plib::systemstate.config.report_rtc_scripts =
      elem.remove_bool( "ReportRunToCompletionScripts", true );
  Plib::systemstate.config.report_critical_scripts =
      elem.remove_bool( "ReportCriticalScripts", true );
  Plib::systemstate.config.report_missing_configs =
      elem.remove_bool( "ReportMissingConfigs", true );
  Plib::systemstate.config.max_clients = elem.remove_ushort( "MaximumClients", 300 );
  Plib::systemstate.config.character_slots = elem.remove_ushort( "CharacterSlots", 5 );
  Plib::systemstate.config.max_clients_bypass_cmdlevel =
      elem.remove_ushort( "MaximumClientsBypassCmdLevel", 1 );
  Plib::systemstate.config.allow_multi_clients_per_account =
      elem.remove_bool( "AllowMultiClientsPerAccount", false );
  Plib::systemstate.config.minidump_type = elem.remove_string( "MiniDumpType", "variable" );
  Plib::systemstate.config.retain_cleartext_passwords =
      elem.remove_bool( "RetainCleartextPasswords", false );
  Plib::systemstate.config.discard_old_events = elem.remove_bool( "DiscardOldEvents", false );
  Clib::LogfileTimestampEveryLine =
      elem.remove_bool( "TimestampEveryLine", false );  // clib/logfacility.h bool
  Plib::systemstate.config.use_single_thread_login =
      elem.remove_bool( "UseSingleThreadLogin", false );
  Plib::systemstate.config.disable_nagle = elem.remove_bool( "DisableNagle", false );
  Plib::systemstate.config.show_realm_info = elem.remove_bool( "ShowRealmInfo", false );

  Plib::systemstate.config.enforce_mount_objtype = elem.remove_bool( "EnforceMountObjtype", false );
  Plib::systemstate.config.single_thread_decay = elem.remove_bool( "SingleThreadDecay", false );
  Plib::systemstate.config.thread_decay_statistics =
      elem.remove_bool( "ThreadDecayStatistics", false );

  // These warnings disable the logging of suspicious acts
  Plib::systemstate.config.show_warning_gump = elem.remove_bool( "ShowWarningGump", true );
  Plib::systemstate.config.show_warning_item = elem.remove_bool( "ShowWarningItem", true );
  Plib::systemstate.config.show_warning_cursor_seq =
      elem.remove_bool( "ShowWarningCursorSequence", true );

  // store the configuration for the reporting system in the ExceptionParser
  bool reportingActive = elem.remove_bool( "ReportCrashsAutomatically", false );
  std::string reportingAdminEmail = elem.remove_string( "ReportAdminEmail", "" );
  std::string reportingServer = elem.remove_string( "ReportServer", "polserver.com" );
  std::string reportingUrl = elem.remove_string( "ReportURL", "/pol/report_program_abort.php" );
  Pol::Clib::ExceptionParser::configureProgramAbortReportingSystem(
      reportingActive, reportingServer, reportingUrl, reportingAdminEmail );

#ifdef _WIN32
  Clib::MiniDumper::SetMiniDumpType( Plib::systemstate.config.minidump_type );
#endif

  if ( !Plib::systemstate.config.enable_debug_log )
    DISABLE_DEBUGLOG();

  Plib::systemstate.config.debug_level = elem.remove_ushort( "DebugLevel", 0 );

  auto allowed_environmentvariables_access =
      elem.remove_string( "AllowedEnvironmentVariablesAccess", "" );
  Plib::systemstate.config.allowed_environmentvariables_access.clear();
  std::stringstream ss( allowed_environmentvariables_access );
  while ( ss.good() )
  {
    std::string substr;
    std::getline( ss >> std::ws, substr, ',' );
    substr.erase( substr.find_last_not_of( " \t\r\n" ) + 1 );
    Clib::mklowerASCII( substr );
    Plib::systemstate.config.allowed_environmentvariables_access.push_back( substr );
  }

  Plib::systemstate.config.enable_colored_output = elem.remove_bool( "EnableColoredOutput", true );

  /// The profiler needs to gather some data before the pol.cfg file gets loaded, so when it
  /// turns out to be disabled, or when it was enabled before, but is being disabled now,
  /// run "garbage collection" to free the allocated resources
  if ( !Plib::systemstate.config.profile_cprops )
    Core::CPropProfiler::instance().clear();
}

void PolConfig::reload_pol_cfg()
{
  THREAD_CHECKPOINT( tasks, 600 );
  try
  {
    struct stat newst;
    stat( "pol.cfg", &newst );

    if ( ( newst.st_mtime != PolConfig::pol_cfg_stat.st_mtime ) &&
         ( newst.st_mtime < time( nullptr ) - 10 ) )
    {
      POLLOG_INFO << "Reloading pol.cfg...";
      memcpy( &PolConfig::pol_cfg_stat, &newst, sizeof PolConfig::pol_cfg_stat );

      PolConfig::read_pol_config( false );
      POLLOG_INFO << "Done!\n";
    }
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERROR << "Error rereading pol.cfg: " << ex.what() << "\n";
  }
  THREAD_CHECKPOINT( tasks, 699 );
}

bool PolConfig::report_program_aborts()
{
  return Clib::ExceptionParser::programAbortReporting();
}
}  // namespace Core
}  // namespace Pol
