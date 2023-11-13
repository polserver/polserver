/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/12/02 Turley:    added MaxTileID -Tomi
 * - 2009/12/04 Turley:    cleanup "MasterKey1","MasterKey2","ClientVersion","KeyFile" - Tomi
 * - 2010/02/04 Turley:    polcfg.discard_old_events discards oldest event if queue is full
 */


#ifndef POLCFG_H
#define POLCFG_H

#include <atomic>
#include <string>
#include <vector>

#if !defined( _WIN32 )
#include <sys/stat.h>
#include <sys/types.h>
#endif
namespace Pol
{
namespace Core
{
struct PolConfig
{
  std::string uo_datafile_root;
  std::string world_data_path;
  std::string realm_data_path;
  std::string pidfile_path;
  std::atomic<bool> verbose;
  unsigned short loglevel;  // 0=nothing 10=lots
  unsigned short select_timeout_usecs;
  unsigned short loginserver_timeout_mins;
  bool watch_rpm;
  bool watch_sysload;
  bool log_sysload;
  bool watch_mapcache;
  bool check_integrity;
  bool inhibit_saves;
  bool log_script_cycles;
  bool count_resource_tiles;

  bool web_server;
  unsigned short web_server_port;
  bool web_server_local_only;
  unsigned short web_server_debug;
  std::string web_server_password;
  bool profile_cprops;
  bool cache_interactive_scripts;
  bool show_speech_colors;
  bool require_spellbooks;
  bool enable_secure_trading;
  unsigned int runaway_script_threshold;
  bool ignore_load_errors;
  std::atomic<unsigned short> min_cmdlvl_ignore_inactivity;
  std::atomic<unsigned short> inactivity_warning_timeout;
  std::atomic<unsigned short> inactivity_disconnect_timeout;
  std::atomic<unsigned short> min_cmdlevel_to_login;
  unsigned int max_tile_id;
  unsigned int max_objtype;

  unsigned short max_clients;
  unsigned short character_slots;
  unsigned short max_clients_bypass_cmdlevel;
  bool allow_multi_clients_per_account;
  bool display_unknown_packets;
  bool exp_los_checks_map;
  bool enable_debug_log;

  unsigned short debug_port;
  std::string debug_password;
  bool debug_local_only;

  bool log_traces_when_stuck;

  unsigned short debug_level;
  bool report_rtc_scripts;
  bool report_critical_scripts;
  bool report_missing_configs;
  bool retain_cleartext_passwords;
  bool discard_old_events;

  int shutdown_save_type;  // either SAVE_FULL or SAVE_INCREMENTAL
  int assertion_shutdown_save_type;

  std::string minidump_type;

  int account_save;
  bool use_single_thread_login;

  bool disable_nagle;
  bool show_realm_info;
  bool enforce_mount_objtype;
  bool single_thread_decay;
  bool thread_decay_statistics;

  bool show_warning_gump;
  bool show_warning_item;
  bool show_warning_cursor_seq;
  bool show_warning_boat_move;

  /**
   * @brief Returns true if program aborts are reported
   */
  bool report_program_aborts();
  /**
   * crash reporting system with some early default values
   */
  std::string report_admin_email;
  std::string report_server;
  std::string report_url;

  std::vector<std::string> allowed_environmentvariables_access;

  bool enable_colored_output;

  static void read_pol_config( bool initial_load );
  static struct stat pol_cfg_stat;
  static void reload_pol_cfg();
};
}  // namespace Core
}  // namespace Pol
#endif
