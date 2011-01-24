/*
History
=======
2007/06/17 Shinigami: added config.world_data_path
2009/12/02 Turley:    added MaxTileID -Tomi
2009/12/04 Turley:    cleanup "MasterKey1","MasterKey2","ClientVersion","KeyFile" - Tomi
2010/02/04 Turley:    polcfg.discard_old_events discards oldest event if queue is full

Notes
=======

*/

#ifndef POLCFG_H
#define POLCFG_H

#include <string>
#include "crypt/cryptkey.h"

struct PolConfig {
	std::string uo_datafile_root;
	std::string world_data_path;
	std::string realm_data_path;
	std::string pidfile_path;
	unsigned short listen_port;
	bool verbose;
	unsigned short loglevel; // 0=nothing 10=lots
	unsigned short select_timeout_usecs;
	bool watch_rpm;
	bool watch_sysload;
	bool log_sysload;
	bool watch_mapcache;
	bool check_integrity;
	bool inhibit_saves;
	bool log_script_cycles;
	bool count_resource_tiles;
	TCryptInfo client_encryption_version;
	unsigned short multithread;
	bool web_server;
	unsigned short web_server_port;
	bool web_server_local_only;
	unsigned short web_server_debug;
	std::string web_server_password;
	bool cache_interactive_scripts;
	bool show_speech_colors;
	bool require_spellbooks;
	bool enable_secure_trading;
	unsigned int runaway_script_threshold;
	bool ignore_load_errors;
	unsigned short min_cmdlvl_ignore_inactivity;
	unsigned short inactivity_warning_timeout;
	unsigned short inactivity_disconnect_timeout;
	unsigned short min_cmdlevel_to_login;
	unsigned short max_tile_id;

	unsigned short max_clients;
	unsigned short character_slots;
	unsigned short max_clients_bypass_cmdlevel;
	bool display_unknown_packets;
	bool exp_los_checks_map;
	bool enable_debug_log;
	
	unsigned short debug_port;
	std::string debug_password;
	bool debug_local_only;

	unsigned short debug_level;
	bool report_rtc_scripts;
	bool report_critical_scripts;
	bool report_missing_configs;
	bool retain_cleartext_passwords;
	bool discard_old_events;

	int shutdown_save_type; // either SAVE_FULL or SAVE_INCREMENTAL
	int assertion_shutdown_save_type;

	std::string minidump_type;
};

extern PolConfig config;

void read_pol_config( bool initial_load );

#endif
