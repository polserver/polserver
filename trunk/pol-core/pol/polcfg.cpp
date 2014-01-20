/*
History
=======
2007/06/17 Shinigami: added config.world_data_path
2009/08/06 MuadDib:   Removed PasswordOnlyHash support
                      ClearTextPasswords now default is False.
2009/10/14 Turley:    added bool LogfileTimestampEveryLine
2009/12/02 Turley:    added MaxTileID -Tomi
2009/12/04 Turley:    cleanup "MasterKey1","MasterKey2","ClientVersion","KeyFile" - Tomi
2010/02/04 Turley:    polcfg.discard_old_events discards oldest event if queue is full

Notes
=======

*/


#include "../clib/stl_inc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../bscript/config.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/dirlist.h"
#include "../clib/mdump.h"
#include "../clib/passert.h"
#include "../clib/strutil.h"
#include "../clib/logfacility.h"

#include "core.h"
#include "objtype.h"
#include "polcfg.h"
#include "polsig.h"
#include "schedule.h"
#include "tasks.h"

#include "crypt/cryptkey.h"
namespace Pol {
  namespace Core {
	PolConfig config;
	extern unsigned int cfg_masterkey1;
	extern unsigned int cfg_masterkey2;

	static struct stat pol_cfg_stat;

	void read_pol_config( bool initial_load )
	{
	  Clib::ConfigFile cf( "pol.cfg" );
      Clib::ConfigElem elem;

	  cf.readraw( elem );

	  if ( initial_load )
	  {
		stat( "pol.cfg", &pol_cfg_stat );

		// these config options can't change after startup.
		config.uo_datafile_root = elem.remove_string( "UoDataFileRoot" );
        config.uo_datafile_root = Clib::normalized_dir_form( config.uo_datafile_root );

		config.world_data_path = elem.remove_string( "WorldDataPath", "data/" );
        config.world_data_path = Clib::normalized_dir_form( config.world_data_path );

		config.realm_data_path = elem.remove_string( "RealmDataPath", "realm/" );
        config.realm_data_path = Clib::normalized_dir_form( config.realm_data_path );

		config.pidfile_path = elem.remove_string( "PidFilePath", "./" );
        config.pidfile_path = Clib::normalized_dir_form( config.pidfile_path );

		config.listen_port = elem.remove_ushort( "ListenPort", 0 );
		config.check_integrity = true; // elem.remove_bool( "CheckIntegrity", true );
		config.count_resource_tiles = elem.remove_bool( "CountResourceTiles", false );
		config.multithread = elem.remove_ushort( "Multithread", 0 );
		config.web_server = elem.remove_bool( "WebServer", false );
		config.web_server_port = elem.remove_ushort( "WebServerPort", 8080 );

		unsigned short max_tile = elem.remove_ushort( "MaxTileID", UOBJ_DEFAULT_MAX );

		if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX )
		  config.max_tile_id = UOBJ_DEFAULT_MAX;
		else
		  config.max_tile_id = max_tile;

		unsigned int max_obj = elem.remove_unsigned( "MaxObjtype", EXTOBJ_HIGHEST_DEFAULT );
		if ( max_obj < EXTOBJ_HIGHEST_DEFAULT || max_obj > 0xFFFFFFFF )
		  config.max_objtype = EXTOBJ_HIGHEST_DEFAULT;
		else
		  config.max_objtype = max_obj;

		config.ignore_load_errors = elem.remove_bool( "IgnoreLoadErrors", false );

		config.debug_port = elem.remove_ushort( "DebugPort", 0 );

		config.account_save = elem.remove_int( "AccountDataSave", -1 );
		if ( config.account_save > 0 )
		{
		  write_account_task.set_secs( config.account_save );
		  write_account_task.start();
		}

	  }
	  config.verbose = elem.remove_bool( "Verbose", false );
	  config.watch_mapcache = elem.remove_bool( "WatchMapCache", false );
	  config.loglevel = elem.remove_ushort( "LogLevel", 0 );
	  config.select_timeout_usecs = elem.remove_ushort( "SelectTimeout", 10 );
	  config.watch_rpm = elem.remove_bool( "WatchRpm", false );
	  config.watch_sysload = elem.remove_bool( "WatchSysLoad", false );
	  config.log_sysload = elem.remove_bool( "LogSysLoad", false );
	  config.inhibit_saves = elem.remove_bool( "InhibitSaves", false );
	  config.log_script_cycles = elem.remove_bool( "LogScriptCycles", false );
	  config.web_server_local_only = elem.remove_bool( "WebServerLocalOnly", true );
	  config.web_server_debug = elem.remove_ushort( "WebServerDebug", 0 );
	  config.web_server_password = elem.remove_string( "WebServerPassword", "" );

	  config.cache_interactive_scripts = elem.remove_bool( "CacheInteractiveScripts", true );
	  config.show_speech_colors = elem.remove_bool( "ShowSpeechColors", false );
	  config.require_spellbooks = elem.remove_bool( "RequireSpellbooks", true );

	  config.enable_secure_trading = elem.remove_bool( "EnableSecureTrading", false );
	  config.runaway_script_threshold = elem.remove_ulong( "RunawayScriptThreshold", 5000 );

	  config.min_cmdlvl_ignore_inactivity = elem.remove_ushort( "MinCmdLvlToIgnoreInactivity", 1 );
	  config.inactivity_warning_timeout = elem.remove_ushort( "InactivityWarningTimeout", 4 );
	  config.inactivity_disconnect_timeout = elem.remove_ushort( "InactivityDisconnectTimeout", 5 );

	  config.min_cmdlevel_to_login = elem.remove_ushort( "MinCmdlevelToLogin", 0 );

	  Bscript::escript_config.max_call_depth = elem.remove_ulong( "MaxCallDepth", 100 );
      Clib::passert_disabled = !elem.remove_bool( "EnableAssertions", true );
      Clib::passert_dump_stack = elem.remove_bool( "DumpStackOnAssertionFailure", false );

	  string tmp = elem.remove_string( "AssertionFailureAction", "abort" );
      if ( Clib::strlower( tmp ) == "abort" )
	  {
        Clib::passert_shutdown = false;
        Clib::passert_nosave = false;
        Clib::passert_abort = true;
		config.assertion_shutdown_save_type = SAVE_FULL; // should never come into play
	  }
      else if ( Clib::strlower( tmp ) == "continue" )
	  {
        Clib::passert_shutdown = false;
        Clib::passert_nosave = false;
        Clib::passert_abort = false;
		config.assertion_shutdown_save_type = SAVE_FULL; // should never come into play
	  }
      else if ( Clib::strlower( tmp ) == "shutdown" )
	  {
        Clib::passert_shutdown = true;
        Clib::passert_nosave = false;
        Clib::passert_abort = false;
		config.assertion_shutdown_save_type = SAVE_FULL;
	  }
      else if ( Clib::strlower( tmp ) == "shutdown-nosave" )
	  {
        Clib::passert_shutdown = true;
        Clib::passert_nosave = true;
        Clib::passert_abort = false;
		config.assertion_shutdown_save_type = SAVE_FULL; // should never come into play
	  }
      else if ( Clib::strlower( tmp ) == "shutdown-save-full" )
	  {
        Clib::passert_shutdown = true;
        Clib::passert_nosave = false;
        Clib::passert_abort = false;
		config.assertion_shutdown_save_type = SAVE_FULL;
	  }
      else if ( Clib::strlower( tmp ) == "shutdown-save-incremental" )
	  {
        Clib::passert_shutdown = true;
        Clib::passert_nosave = false;
        Clib::passert_abort = false;
		config.assertion_shutdown_save_type = SAVE_INCREMENTAL;
	  }
	  else
	  {
        Clib::passert_shutdown = false;
        Clib::passert_abort = true;
		config.assertion_shutdown_save_type = SAVE_FULL; // should never come into play
        POLLOG_ERROR.Format("Unknown pol.cfg AssertionFailureAction value: {} (expected abort, continue, shutdown, or shutdown-nosave)\n")<< tmp;
	  }

	  tmp = elem.remove_string( "ShutdownSaveType", "full" );
      if ( Clib::strlower( tmp ) == "full" )
	  {
		config.shutdown_save_type = SAVE_FULL;
	  }
      else if ( Clib::strlower( tmp ) == "incremental" )
	  {
		config.shutdown_save_type = SAVE_INCREMENTAL;
	  }
	  else
	  {
		config.shutdown_save_type = SAVE_FULL;
        POLLOG_ERROR.Format( "Unknown pol.cfg ShutdownSaveType value: {} (expected full or incremental)\n" ) << tmp;
	  }

	  CalculateCryptKeys( elem.remove_string( "ClientEncryptionVersion", "none" ), config.client_encryption_version );

	  config.display_unknown_packets = elem.remove_bool( "DisplayUnknownPackets", false );
	  config.exp_los_checks_map = elem.remove_bool( "ExpLosChecksMap", true );
	  config.enable_debug_log = elem.remove_bool( "EnableDebugLog", false );
	  config.debug_password = elem.remove_string( "DebugPassword", "" );
	  config.debug_local_only = elem.remove_bool( "DebugLocalOnly", true );

	  config.report_rtc_scripts = elem.remove_bool( "ReportRunToCompletionScripts", true );
	  config.report_critical_scripts = elem.remove_bool( "ReportCriticalScripts", true );
	  config.report_missing_configs = elem.remove_bool( "ReportMissingConfigs", true );
	  config.max_clients = elem.remove_ushort( "MaximumClients", 300 );
	  config.character_slots = elem.remove_ushort( "CharacterSlots", 5 );
	  config.max_clients_bypass_cmdlevel = elem.remove_ushort( "MaximumClientsBypassCmdLevel", 1 );
	  config.minidump_type = elem.remove_string( "MiniDumpType", "variable" );
	  config.retain_cleartext_passwords = elem.remove_bool( "RetainCleartextPasswords", false );
	  config.discard_old_events = elem.remove_bool( "DiscardOldEvents", false );
      Clib::LogfileTimestampEveryLine = elem.remove_bool( "TimestampEveryLine", false ); // clib/logfacility.h bool
	  config.use_single_thread_login = elem.remove_bool( "UseSingleThreadLogin", false );
	  config.disable_nagle = elem.remove_bool( "DisableNagle", false );

#ifdef _WIN32
      Clib::MiniDumper::SetMiniDumpType( config.minidump_type );
#endif

      if ( !config.enable_debug_log )
        DISABLE_DEBUGLOG();

	  config.debug_level = elem.remove_ushort( "DebugLevel", 0 );
	}

	void reload_pol_cfg( void )
	{
	  THREAD_CHECKPOINT( tasks, 600 );
	  try
	  {
		struct stat newst;
		stat( "pol.cfg", &newst );

		if ( ( newst.st_mtime != pol_cfg_stat.st_mtime ) &&
			 ( newst.st_mtime < time( NULL ) - 10 ) )
		{
          POLLOG_INFO << "Reloading pol.cfg...";
		  memcpy( &pol_cfg_stat, &newst, sizeof pol_cfg_stat );

		  read_pol_config( false );
          POLLOG_INFO << "Done!\n";

		}
	  }
	  catch ( std::exception& ex )
	  {
        POLLOG_ERROR << "Error rereading pol.cfg: " << ex.what( ) << "\n";
	  }
	  THREAD_CHECKPOINT( tasks, 699 );
	}

	PeriodicTask reload_pol_cfg_task( reload_pol_cfg, 30, "LOADPOLCFG" );
  }
}