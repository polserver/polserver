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
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <time.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "objtype.h"
#include "uoinstallfinder.h"

namespace Pol::Plib
{
struct stat PolConfig::pol_cfg_stat;

void PolConfig::read( bool initial_load )
{
  Clib::ConfigFile cf( "pol.cfg" );
  Clib::ConfigElem elem;

  cf.readraw( elem );

  if ( initial_load )
  {
    stat( "pol.cfg", &pol_cfg_stat );

    // these config options can't change after startup.
    uo_datafile_root = Plib::UOInstallFinder::remove_elem( elem );
    uo_datafile_root = Clib::normalized_dir_form( uo_datafile_root );

    world_data_path = elem.remove_string( "WorldDataPath", "data/" );
    world_data_path = Clib::normalized_dir_form( world_data_path );

    realm_data_path = elem.remove_string( "RealmDataPath", "realm/" );
    realm_data_path = Clib::normalized_dir_form( realm_data_path );

    pidfile_path = elem.remove_string( "PidFilePath", "./" );
    pidfile_path = Clib::normalized_dir_form( pidfile_path );

    check_integrity = true;  // elem.remove_bool( "CheckIntegrity", true );
    count_resource_tiles = elem.remove_bool( "CountResourceTiles", false );
    web_server = elem.remove_bool( "WebServer", false );
    web_server_port = elem.remove_ushort( "WebServerPort", 8080 );

    unsigned short max_tile = elem.remove_ushort( "MaxTileID", 0 );
    if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX &&
         max_tile != 0 )
    {
      max_tile = UOBJ_DEFAULT_MAX;
    }
    max_tile_id = max_tile;

    max_anim_id = elem.remove_ushort( "MaxAnimID", UOBJ_MAX_ANIMS );

    unsigned int max_obj = elem.remove_unsigned( "MaxObjtype", EXTOBJ_HIGHEST_DEFAULT );
    if ( max_obj < EXTOBJ_HIGHEST_DEFAULT || max_obj > 0xFFFFFFFF )
      max_objtype = EXTOBJ_HIGHEST_DEFAULT;
    else
      max_objtype = max_obj;

    ignore_load_errors = elem.remove_bool( "IgnoreLoadErrors", false );

    debug_port = elem.remove_ushort( "DebugPort", 0 );
    dap_debug_port = elem.remove_ushort( "DAPDebugPort", 0 );
    debug_password = elem.remove_string( "DebugPassword", "" );
    debug_local_only = elem.remove_bool( "DebugLocalOnly", true );

    account_save = elem.remove_int( "AccountDataSave", -1 );
    default_priority =
        Clib::clamp_convert<unsigned char>( elem.remove_ushort( "DefaultPriority", 10 ) );
  }
  verbose = elem.remove_bool( "Verbose", false );
  watch_mapcache = elem.remove_bool( "WatchMapCache", false );
  loglevel = elem.remove_ushort( "LogLevel", 0 );
  loginserver_select_timeout_msecs = elem.remove_ushort( "LoginServerSelectTimeout", 1 );
  loginserver_timeout_mins = elem.remove_ushort( "LoginServerTimeout", 10 );
  watch_rpm = elem.remove_bool( "WatchRpm", false );
  watch_sysload = elem.remove_bool( "WatchSysLoad", false );
  log_sysload = elem.remove_bool( "LogSysLoad", false );
  inhibit_saves = elem.remove_bool( "InhibitSaves", false );
  log_script_cycles = elem.remove_bool( "LogScriptCycles", false );
  web_server_local_only = elem.remove_bool( "WebServerLocalOnly", true );
  web_server_debug = elem.remove_ushort( "WebServerDebug", 0 );
  web_server_password = elem.remove_string( "WebServerPassword", "" );

  profile_cprops = elem.remove_bool( "ProfileCProps", false );

  cache_interactive_scripts = elem.remove_bool( "CacheInteractiveScripts", true );
  show_speech_colors = elem.remove_bool( "ShowSpeechColors", false );
  require_spellbooks = elem.remove_bool( "RequireSpellbooks", true );

  enable_secure_trading = elem.remove_bool( "EnableSecureTrading", false );
  runaway_script_threshold = elem.remove_ulong( "RunawayScriptThreshold", 5000 );

  min_cmdlvl_ignore_inactivity = elem.remove_ushort( "MinCmdLvlToIgnoreInactivity", 1 );
  inactivity_warning_timeout = elem.remove_ushort( "InactivityWarningTimeout", 4 );
  inactivity_disconnect_timeout = elem.remove_ushort( "InactivityDisconnectTimeout", 5 );

  min_cmdlevel_to_login = elem.remove_ushort( "MinCmdlevelToLogin", 0 );

  max_call_depth = elem.remove_ulong( "MaxCallDepth", 100 );
  passert_dump_stack = elem.remove_bool( "DumpStackOnAssertionFailure", false );
  passert_failure_action =
      Clib::strlowerASCII( elem.remove_string( "AssertionFailureAction", "abort" ) );

  display_unknown_packets = elem.remove_bool( "DisplayUnknownPackets", false );
  exp_los_checks_map = elem.remove_bool( "ExpLosChecksMap", true );
  enable_debug_log = elem.remove_bool( "EnableDebugLog", false );

  log_traces_when_stuck = elem.remove_bool( "ThreadStacktracesWhenStuck", false );

  report_rtc_scripts = elem.remove_bool( "ReportRunToCompletionScripts", true );
  report_critical_scripts = elem.remove_bool( "ReportCriticalScripts", true );
  report_missing_configs = elem.remove_bool( "ReportMissingConfigs", true );
  max_clients = elem.remove_ushort( "MaximumClients", 300 );
  character_slots = Clib::clamp_convert<u8>( elem.remove_ushort( "CharacterSlots", 5 ) );
  max_clients_bypass_cmdlevel = elem.remove_ushort( "MaximumClientsBypassCmdLevel", 1 );
  allow_multi_clients_per_account = elem.remove_bool( "AllowMultiClientsPerAccount", false );
  minidump_type = elem.remove_string( "MiniDumpType", "variable" );
  retain_cleartext_passwords = elem.remove_bool( "RetainCleartextPasswords", false );
  discard_old_events = elem.remove_bool( "DiscardOldEvents", false );
  logfile_timestamp_everyline = elem.remove_bool( "TimestampEveryLine", false );
  use_single_thread_login = elem.remove_bool( "UseSingleThreadLogin", true );
  loginserver_disconnect_unknown_pkts =
      elem.remove_bool( "LoginServerDisconnectUnknownPkts", false );
  disable_nagle = elem.remove_bool( "DisableNagle", false );
  show_realm_info = elem.remove_bool( "ShowRealmInfo", false );

  enforce_mount_objtype = elem.remove_bool( "EnforceMountObjtype", false );
  thread_decay_statistics = elem.remove_bool( "ThreadDecayStatistics", false );

  // These warnings disable the logging of suspicious acts
  show_warning_gump = elem.remove_bool( "ShowWarningGump", true );
  show_warning_item = elem.remove_bool( "ShowWarningItem", true );
  show_warning_cursor_seq = elem.remove_bool( "ShowWarningCursorSequence", true );
  show_warning_boat_move = elem.remove_bool( "ShowWarningBoatMove", true );

  report_active = elem.remove_bool( "ReportCrashsAutomatically", false );
  report_admin_email = elem.remove_string( "ReportAdminEmail", "" );
  report_server = elem.remove_string( "ReportServer", "polserver.com" );
  report_url = elem.remove_string( "ReportURL", "/pol/report_program_abort.php" );

  debug_level = elem.remove_ushort( "DebugLevel", 0 );

  auto allowed_environmentvariables_access_str =
      elem.remove_string( "AllowedEnvironmentVariablesAccess", "" );
  allowed_environmentvariables_access.clear();
  std::stringstream ss( allowed_environmentvariables_access_str );
  while ( ss.good() )
  {
    std::string substr;
    std::getline( ss >> std::ws, substr, ',' );
    substr.erase( substr.find_last_not_of( " \t\r\n" ) + 1 );
    Clib::mklowerASCII( substr );
    allowed_environmentvariables_access.push_back( substr );
  }

  enable_colored_output = elem.remove_bool( "EnableColoredOutput", true );
}

}  // namespace Pol::Plib
