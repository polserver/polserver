/** @file
 *
 * @par History
 * - 2005/02/23 Shinigami: ServSpecOpt DecayItems to enable/disable item decay
 * - 2005/04/03 Shinigami: send_feature_enable() call moved from start_client_char()
 *                         to send_start() to send before char selection
 * - 2005/04/04 Shinigami: added can_delete_character( chr, delete_by )
 * - 2005/06/15 Shinigami: ServSpecOpt UseWinLFH to enable/disable Windows XP/2003 low-fragmentation
 * Heap
 *                         added Check_for_WinXP_or_Win2003() and low_fragmentation_Heap()
 * - 2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
 * - 2005/07/01 Shinigami: removed Check_for_WinXP_or_Win2003() and transformed call of
 *                         Use_low_fragmentation_Heap() into Run-Time Dynamic Linking
 * - 2005/10/13 Shinigami: added Check_libc_version() and printing a Warning if libc is to old
 * - 2005/11/25 Shinigami: PKTBI_BF::TYPE_SESPAM will not block Inactivity-Check
 * - 2005/11/28 MuadDib:   Created check_inactivity() bool function to handle checking packets
 *                         for ones to be considered "ignored" for inactivity. Returns true if
 *                         the packet was one to be ignored.
 * - 2005/11/28 MuadDib:   Implemented check_inactivity() function in appropriate place.
 * - 2006/03/01 MuadDib:   Added connect = true to start_client_char so char creation can use.
 * - 2006/03/03 MuadDib:   Moved all instances of connected = true to start_client_char.
 * - 2006/06/03 Shinigami: added little bit more logging @ pthread_create
 * - 2006/06/05 Shinigami: added little bit more logging @ Client disconnects by Core
 * - 2006/07/05 Shinigami: moved MakeDirectory("log") a little bit up
 * - 2006/10/07 Shinigami: FreeBSD fix - changed some __linux__ to __unix__
 * - 2007/03/08 Shinigami: added pthread_exit and _endhreadex to close threads
 * - 2007/05/06 Shinigami: smaller bugfix in Check_libc_version()
 * - 2007/06/17 Shinigami: Pergon-Linux-Release generates file "pol.pid"
 * - 2007/07/08 Shinigami: added UO:KR login process
 * - 2008/07/08 Turley:    removed Checkpoint "initializing random number generator"
 * - 2008/12/17 MuadDub:   Added check when loading Realms for no realms existing.
 * - 2009/01/19 Nando:     added unload_aux_services() and unload_packages() to the shutdown cleanup
 * - 2009/1/24  MuadDib:   Added read_bannedips_config() and checkpoint for it after loading of
 * pol.cfg
 * - 2009/07/23 MuadDib:   Updates for MSGOUT naming.
 * - 2009/07/31 MuadDib:   xmain_inner(): Force Client Disconnect to initiate cleanup of clients and
 * chars, after shutdown,
 *                         before other cleanups.
 * - 2009/08/01 MuadDib:   Removed send_tech_stuff(), send_betaclient_BF(), just_ignore_message(),
 * and ignore_69() due to not used or obsolete.
 * - 2009/08/03 MuadDib:   Renaming of MSG_HANDLER_6017 and related, to MSG_HANDLER_V2 for better
 * description
 *                         Renamed secondary handler class to *_V2 for naming convention
 * - 2009/08/14 Turley:    fixed definition of PKTIN_5D
 * - 2009/08/19 Turley:    PKTIN_5D clientflag saved in client->UOExpansionFlagClient
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Changes for multi related source file relocation
 * - 2009/09/15 MuadDib:   Multi registration/unregistration support added.
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/09/22 MuadDib:   Fix for lightlevel resets in client during login.
 * - 2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi
 * - 2009/12/04 Turley:    Crypto cleanup - Tomi
 * - 2010/01/22 Turley:    Speedhack Prevention System
 * - 2010/03/28 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within
 * decay_thread_shadow
 * - 2011/11/12 Tomi:    Added extobj.cfg
 */

#include "pol.h"

#include <errno.h>

#include "pol_global_config.h"

#include "../bscript/bobject.h"
#include "../bscript/escriptv.h"
#include "../clib/Debugging/ExceptionParser.h"
#include "../clib/Program/ProgramConfig.h"
#include "../clib/clib_endian.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/kbhit.h"
#include "../clib/logfacility.h"
#include "../clib/network/sockets.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../clib/streamsaver.h"
#include "../clib/threadhelp.h"
#include "../clib/timer.h"
#include "../clib/tracebuf.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "accounts/account.h"
#include "checkpnt.h"
#include "console.h"
#include "core.h"
#include "decay.h"
#include "extobj.h"
#include "fnsearch.h"
#include "gameclck.h"
#include "globals/network.h"
#include "globals/object_storage.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "guardrgn.h"
#include "item/armor.h"
#include "item/equipmnt.h"
#include "item/itemdesc.h"
#include "loadunld.h"
#include "menu.h"
#include "miscrgn.h"
#include "mobile/charactr.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "musicrgn.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "network/clientthread.h"
#include "network/clienttransmit.h"
#include "network/cliface.h"
#include "network/packethelper.h"
#include "network/packethooks.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "network/sockio.h"
#include "party.h"
#include "polcfg.h"
#include "polclock.h"
#include "poldbg.h"
#include "polsem.h"
#include "polsig.h"
#include "polwww.h"
#include "profile.h"
#include "realms/WorldChangeReasons.h"
#include "realms/realm.h"
#include "savedata.h"
#include "scrdef.h"
#include "scrsched.h"
#include "sqlscrobj.h"
#include "ssopt.h"
#include "testing/poltest.h"
#include "ufunc.h"
#include "uimport.h"
#include "uoclient.h"
#include "uoscrobj.h"
#include "uworld.h"
#include <format/format.h>

#ifndef NDEBUG
#include "containr.h"
#include "mobile/npc.h"
#endif

#ifdef _WIN32
#include <process.h>

#include "../clib/mdump.h"
#endif

#ifdef __linux__
#include <gnu/libc-version.h>
#endif


#include <cstdio>
#include <cstring>
#include <exception>
#include <iosfwd>
#include <string>

#ifdef _MSC_VER
#pragma warning( disable : 4127 )  // conditional expression is constant (needed because of FD_SET)
#endif

namespace Pol
{
namespace Bscript
{
void display_executor_instances();
void display_bobjectimp_instances();
}  // namespace Bscript
namespace Items
{
void load_intrinsic_weapons();
void allocate_intrinsic_weapon_serials();
}  // namespace Items
namespace Network
{
void load_aux_services();
void start_aux_services();
void read_bannedips_config( bool initial_load );
}  // namespace Network
namespace Core
{
void cancel_all_trades();
void load_system_hooks();
bool load_realms();
void InitializeSystemTrayHandling();
void ShutdownSystemTrayHandling();
void start_uo_client_listeners( void );
void start_tasks();


using namespace threadhelp;

#define CLIENT_CHECKPOINT( x ) client->checkpoint = x

void send_startup( Network::Client* client )
{
  Mobile::Character* chr = client->chr;
  Network::PktHelper::PacketOut<Network::PktOut_1B> msg;
  msg->Write<u32>( chr->serial_ext );
  msg->offset += 4;  // u8 unk5, unk6, unk7, unk8
  msg->WriteFlipped<u16>( chr->graphic );
  msg->WriteFlipped<u16>( chr->x );
  msg->WriteFlipped<u16>( chr->y );
  msg->offset++;  // u8 unk_15
  msg->Write<s8>( chr->z );
  msg->Write<u8>( chr->facing );
  msg->offset += 3;  // u8 unk18,unk19,unk20
  msg->Write<u8>( 0x7Fu );
  msg->offset++;     // u8 unk22
  msg->offset += 4;  // u16 map_startx, map_starty
  msg->WriteFlipped<u16>( client->chr->realm->width() );
  msg->WriteFlipped<u16>( client->chr->realm->height() );
  msg->offset += 6;  // u8 unk31, unk32, unk33, unk34, unk35, unk36
  msg.Send( client );
}

void send_inrange_items( Network::Client* client )
{
  WorldIterator<ItemFilter>::InVisualRange(
      client->chr, [&]( Items::Item* item ) { send_item( client, item ); } );
}

void send_inrange_multis( Network::Client* client )
{
  WorldIterator<MultiFilter>::InVisualRange(
      client->chr, [&]( Multi::UMulti* multi ) { send_multi( client, multi ); } );
}

void textcmd_startlog( Network::Client* client );
void textcmd_stoplog( Network::Client* client );
void start_client_char( Network::Client* client )
{
  client->ready = true;
  client->chr->connected( true );

  // even if this stuff just gets queued, we still want the client to start
  // getting data now.
  client->pause();

  Multi::UMulti* supporting_multi;
  Items::Item* walkon;
  short newz;
  if ( client->chr->realm->walkheight( client->chr, client->chr->x, client->chr->y, client->chr->z,
                                       &newz, &supporting_multi, &walkon ) )
  {
    client->chr->z = static_cast<s8>( newz );
    // FIXME: Need to add Walkon checks for multi right here if type is house.
    if ( supporting_multi != nullptr )
    {
      supporting_multi->register_object( client->chr );
      Multi::UHouse* this_house = supporting_multi->as_house();
      if ( client->chr->registered_house == 0 )
      {
        client->chr->registered_house = supporting_multi->serial;

        if ( this_house != nullptr )
          this_house->walk_on( client->chr );
      }
    }
    else
    {
      if ( client->chr->registered_house > 0 )
      {
        Multi::UMulti* multi = system_find_multi( client->chr->registered_house );
        if ( multi != nullptr )
        {
          multi->unregister_object( client->chr );
        }
        client->chr->registered_house = 0;
      }
    }
    client->chr->position_changed();
  }

  send_startup( client );

  send_realm_change( client, client->chr->realm );
  send_map_difs( client );

  if ( settingsManager.ssopt.core_sends_season )
    send_season_info( client );

  client->chr->lastx = client->chr->lasty = client->chr->lastz = 0;

  client->gd->music_region = gamestate.musicdef->getregion( 0, 0, client->chr->realm );
  client->gd->justice_region = gamestate.justicedef->getregion( 0, 0, client->chr->realm );
  client->gd->weather_region = gamestate.weatherdef->getregion( 0, 0, client->chr->realm );

  send_goxyz( client, client->chr );
  client->chr->check_region_changes();

  client->chr->send_warmode();
  login_complete( client );
  client->chr->tellmove();

  client->chr->check_weather_region_change( true );

  if ( settingsManager.ssopt.core_sends_season )
    send_season_info( client );

  send_objects_newly_inrange( client );

  client->chr->send_highlight();
  send_owncreate( client, client->chr );

  send_goxyz( client, client->chr );

  client->restart();

  client->chr->clear_gotten_item();
  on_loggon_party( client->chr );
  client->chr->send_buffs();


  //  Moved login_complete higher to prevent weather regions from messing up client
  //  spell icon packets(made it look like it was raining spell icons from spellbook if logged
  //  into a weather region with rain.
  //  login_complete(client);
}


void call_chr_scripts( Mobile::Character* chr, const std::string& root_script_ecl,
                       const std::string& pkg_script_ecl, bool offline = false )
{
  ScriptDef sd;
  sd.quickconfig( root_script_ecl );

  if ( sd.exists() )
  {
    call_script( sd, offline ? new Module::EOfflineCharacterRefObjImp( chr )
                             : new Module::ECharacterRefObjImp( chr ) );
  }

  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = *itr;

    sd.quickconfig( pkg, pkg_script_ecl );
    if ( sd.exists() )
    {
      call_script( sd, offline ? new Module::EOfflineCharacterRefObjImp( chr )
                               : new Module::ECharacterRefObjImp( chr ) );
    }
  }
}

void run_logon_script( Mobile::Character* chr )
{
  call_chr_scripts( chr, "scripts/misc/logon.ecl", "logon.ecl" );
}
void run_reconnect_script( Mobile::Character* chr )
{
  call_chr_scripts( chr, "scripts/misc/reconnect.ecl", "reconnect.ecl" );
}
bool can_delete_character( Mobile::Character* chr, int delete_by )
{
  ScriptDef sd;
  sd.quickconfig( "scripts/misc/candelete.ecl" );

  if ( sd.exists() )
  {
    return call_script( sd, new Module::EOfflineCharacterRefObjImp( chr ),
                        new Bscript::BLong( delete_by ) );
  }
  else
  {
    return true;
  }
}
void call_ondelete_scripts( Mobile::Character* chr )
{
  call_chr_scripts( chr, "scripts/misc/ondelete.ecl", "ondelete.ecl", true );
}

// FIXME: Consider moving most of this into a function, so character
// creation can use the same code.
void char_select( Network::Client* client, PKTIN_5D* msg )
{
  bool reconnecting = false;
  int charidx = cfBEu32( msg->charidx );
  if ( ( charidx >= Plib::systemstate.config.character_slots ) || ( client->acct == nullptr ) ||
       ( client->acct->get_character( charidx ) == nullptr ) )
  {
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }

  Mobile::Character* chosen_char = client->acct->get_character( charidx );

  POLLOG.Format( "Account {} selecting character {}\n" )
      << client->acct->name() << chosen_char->name();

  if ( Plib::systemstate.config.min_cmdlevel_to_login > chosen_char->cmdlevel() )
  {
    POLLOG.Format(
        "Account {} with character {} doesn't fit MinCmdlevelToLogin from pol.cfg. Client "
        "disconnected by Core.\n" )
        << client->acct->name() << chosen_char->name();

    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }

  // Dave moved this from login.cpp so client cmdlevel can be checked before denying login
  if ( ( ( std::count_if( networkManager.clients.begin(), networkManager.clients.end(),
                          clientHasCharacter ) ) >= Plib::systemstate.config.max_clients ) &&
       ( chosen_char->cmdlevel() < Plib::systemstate.config.max_clients_bypass_cmdlevel ) )
  {
    POLLOG.Format(
        "To much clients connected. Check MaximumClients and/or MaximumClientsBypassCmdLevel in "
        "pol.cfg.\nAccount {} with character {} Client disconnected by Core.\n" )
        << client->acct->name() << chosen_char->name();

    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }

  if ( chosen_char->client )
  {
    // we're reattaching to a character that is in-game.  If there is still
    // a client attached, disconnect it.

    chosen_char->client->gd->clear();
    chosen_char->client->forceDisconnect();
    chosen_char->client->ready = false;
    chosen_char->client->msgtype_filter = networkManager.disconnected_filter.get();

    // disassociate the objects from each other.
    chosen_char->client->acct = nullptr;
    chosen_char->client->chr = nullptr;

    chosen_char->client = nullptr;
    reconnecting = true;
  }
  else if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
            client->acct->has_active_characters() )
  {
    // We are trying to attach a new character, but AllowMultiCharacters is not set
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }
  else
  {
    // logging in a character that's offline.
    SetCharacterWorldPosition( chosen_char, Realms::WorldChangeReason::PlayerEnter );
    chosen_char->logged_in( true );
  }

  client->chr = chosen_char;
  chosen_char->client = client;
  chosen_char->acct.set( client->acct );

  client->UOExpansionFlagClient = cfBEu32( msg->clientflags );

  client->msgtype_filter = networkManager.game_filter.get();
  start_client_char( client );

  if ( !chosen_char->lastx && !chosen_char->lasty )
  {
    chosen_char->lastx = chosen_char->x;
    chosen_char->lasty = chosen_char->y;
  }

  if ( !reconnecting )
    run_logon_script( chosen_char );
  else
    run_reconnect_script( chosen_char );
}

void send_client_char_data( Mobile::Character* chr, Network::Client* client );
void handle_resync_request( Network::Client* client, PKTBI_22_SYNC* /*msg*/ )
{
  send_goxyz( client, client->chr );

  client->send_pause();  // dave removed force=true 5/10/3, let uoclient.cfg option determine xflow
                         // packets (else this hangs 4.0.0e clients)

  Core::WorldIterator<Core::MobileFilter>::InVisualRange(
      client->chr,
      [&]( Mobile::Character* zonechr ) { send_client_char_data( zonechr, client ); } );

  send_inrange_items( client );
  send_inrange_multis( client );

  client->send_restart();  // dave removed force=true 5/10/3
}

void restart_all_clients()
{
  if ( !networkManager.uoclient_protocol.EnableFlowControlPackets )
    return;
  for ( Clients::iterator itr = networkManager.clients.begin(), end = networkManager.clients.end();
        itr != end; ++itr )
  {
    Network::Client* client = ( *itr );
    if ( client->pause_count )
    {
      client->restart();
    }
  }
}

void polclock_checkin()
{
  stateManager.checkin_clock_times_out_at = polclock() + 30 * POLCLOCKS_PER_SEC;
}

#define clock_t_to_ms( x ) ( x )

void tasks_thread( void )
{
  polclock_t sleeptime;
  bool activity;
  try
  {
    while ( !Clib::exit_signalled )
    {
      THREAD_CHECKPOINT( tasks, 1 );
      {
        PolLock lck;
        polclock_checkin();
        THREAD_CHECKPOINT( tasks, 2 );
        INC_PROFILEVAR( scheduler_passes );
        check_scheduled_tasks( &sleeptime, &activity );
        THREAD_CHECKPOINT( tasks, 3 );
        restart_all_clients();
        THREAD_CHECKPOINT( tasks, 5 );
      }

      THREAD_CHECKPOINT( tasks, 6 );
      if ( activity )
        send_pulse();
      else
        INC_PROFILEVAR( noactivity_scheduler_passes );
      THREAD_CHECKPOINT( tasks, 7 );

      passert( sleeptime > 0 );

      TRACEBUF_ADDELEM( "tasks wait_for_pulse now", polclock() );
      TRACEBUF_ADDELEM( "tasks wait_for_pulse sleeptime", sleeptime );

      THREAD_CHECKPOINT( tasks, 8 );
      tasks_thread_sleep( polclock_t_to_ms( sleeptime ) );
      THREAD_CHECKPOINT( tasks, 9 );
    }
  }
  catch ( const char* msg )
  {
    POLLOG.Format( "Tasks Thread exits due to exception: {}\n" ) << msg;
    throw;
  }
  catch ( std::string& str )
  {
    POLLOG.Format( "Tasks Thread exits due to exception: {}\n" ) << str;
    throw;
  }
  catch ( std::exception& ex )
  {
    POLLOG.Format( "Tasks Thread exits due to exception: {}\n" ) << ex.what();
    throw;
  }
}

void scripts_thread( void )
{
  polclock_t sleeptime;
  bool activity;
  while ( !Clib::exit_signalled )
  {
    THREAD_CHECKPOINT( scripts, 0 );
    {
      PolLock lck;
      polclock_checkin();
      TRACEBUF_ADDELEM( "scripts thread now", polclock() );
      ++stateManager.profilevars.script_passes;
      THREAD_CHECKPOINT( scripts, 1 );

      step_scripts( &sleeptime, &activity );

      THREAD_CHECKPOINT( scripts, 50 );

      restart_all_clients();

      THREAD_CHECKPOINT( scripts, 52 );

      if ( TaskScheduler::is_dirty() )
      {
        THREAD_CHECKPOINT( scripts, 53 );

        wake_tasks_thread();
      }
    }

    if ( activity )
    {
      ++stateManager.profilevars.script_passes_activity;
    }
    else
    {
      ++stateManager.profilevars.script_passes_noactivity;
    }

    if ( sleeptime )
    {
      THREAD_CHECKPOINT( scripts, 54 );

      wait_for_pulse( polclock_t_to_ms( sleeptime ) );

      THREAD_CHECKPOINT( scripts, 55 );
    }
  }
}

template <class T>
inline void Delete( T* p )
{
  delete p;
}

template <class T>
class delete_ob
{
public:
  void operator()( T* p ) { delete p; }
};

void reap_thread( void )
{
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      objStorageManager.objecthash.Reap();
      for ( auto& item : gamestate.dynamic_item_descriptors )
      {
        delete item;
      }
      gamestate.dynamic_item_descriptors.clear();
    }

    threadhelp::thread_sleep_ms( 2000 );
  }
}


void threadstatus_thread( void )
{
  int timeouts_remaining = 1;
  bool sent_wakeups = false;
  // we want this thread to be the last out, so that it can report stuff at shutdown.
  while ( !Clib::exit_signalled || threadhelp::child_threads > 1 )
  {
    if ( is_polclock_paused_at_zero() )
    {
      polclock_t now = polclock();
      if ( now >= stateManager.checkin_clock_times_out_at )
      {
        ERROR_PRINT << "########################################################\n";
        ERROR_PRINT << "No clock movement in 30 seconds.  Dumping thread status.\n";
        stateManager.polsig.report_status_signalled = true;
        stateManager.checkin_clock_times_out_at = now + 30 * POLCLOCKS_PER_SEC;
      }
    }

    if ( stateManager.polsig.report_status_signalled )
    {
      fmt::Writer tmp;
      tmp << "*Thread Info*\n";
      tmp << "Semaphore TID: " << locker << "\n";

      if ( Plib::systemstate.config.log_traces_when_stuck )
        Pol::Clib::ExceptionParser::logAllStackTraces();

      tmp << "Scripts Thread Checkpoint: " << stateManager.polsig.scripts_thread_checkpoint << "\n";
      tmp << "Last Script: " << Clib::scripts_thread_script
          << " PC: " << Clib::scripts_thread_scriptPC << "\n";
      tmp << "Escript Instruction Cycles: " << Bscript::escript_instr_cycles << "\n";
      tmp << "Tasks Thread Checkpoint: " << stateManager.polsig.tasks_thread_checkpoint << "\n";
      tmp << "Active Client Thread Checkpoint: "
          << stateManager.polsig.active_client_thread_checkpoint << "\n";
      tmp << "Number of clients: " << Core::networkManager.clients.size() << "\n";
      for ( const auto& client : Core::networkManager.clients )
        tmp << " " << client->ipaddrAsString() << " "
            << ( client->acct == nullptr ? "prelogin " : client->acct->name() ) << " "
            << client->checkpoint << "\n";
      if ( stateManager.polsig.check_attack_after_move_function_checkpoint )
        tmp << "check_attack_after_move() Checkpoint: "
            << stateManager.polsig.check_attack_after_move_function_checkpoint << "\n";
      tmp << "Current Threads:"
          << "\n";
      ThreadMap::Contents contents;
      threadmap.CopyContents( contents );
      for ( ThreadMap::Contents::const_iterator citr = contents.begin(); citr != contents.end();
            ++citr )
      {
        tmp << ( *citr ).first << " - " << ( *citr ).second << "\n";
      }
      tmp << "Child threads (child_threads): " << threadhelp::child_threads << "\n";
      tmp << "Registered threads (ThreadMap): " << contents.size() << "\n";
      stateManager.polsig.report_status_signalled = false;
      ERROR_PRINT << tmp.str();
    }
    if ( Clib::exit_signalled )
    {
      if ( !sent_wakeups )
      {
        send_pulse();
        wake_tasks_thread();
        networkManager.clientTransmit->Cancel();
#ifdef HAVE_MYSQL
        networkManager.sql_service->stop();
#endif
        sent_wakeups = true;
      }

      --timeouts_remaining;
      if ( timeouts_remaining == 0 )
      {
        INFO_PRINT << "Waiting for " << threadhelp::child_threads << " child threads to exit\n";
        timeouts_remaining = 5;
      }
    }
    pol_sleep_ms( 1000 );
  }
  // cerr << "threadstatus thread exits." << endl;
  signal_catch_thread();
}

void catch_signals_thread( void );
void reload_configuration();

void console_thread( void )
{
#ifndef _WIN32
  Clib::KeyboardHook kb;  // local to have a defined deconstruction to uninstall the hook
#endif
  while ( !Clib::exit_signalled )
  {
    pol_sleep_ms( 1000 );
#ifdef _WIN32
    ConsoleCommand::check_console_commands();
#else
    ConsoleCommand::check_console_commands( &kb );
    if ( stateManager.polsig.reload_configuration_signalled )
    {
      PolLock lck;
      INFO_PRINT << "Reloading configuration...";
      stateManager.polsig.reload_configuration_signalled = false;
      reload_configuration();
      INFO_PRINT << "Done.\n";
    }
#endif
  }
}

void start_threads()
{
  threadmap.Register( thread_pid(), "Main" );

  if ( Plib::systemstate.config.web_server )
    start_http_server();

  checkpoint( "start tasks thread" );
  threadhelp::start_thread( tasks_thread, "Tasks" );
  checkpoint( "start scripts thread" );
  threadhelp::start_thread( scripts_thread, "Scripts" );

  if ( settingsManager.ssopt.decay_items )
  {
    checkpoint( "start decay thread" );
    if ( Plib::systemstate.config.single_thread_decay )
    {
      threadhelp::start_thread( decay_single_thread, "Decay", nullptr );
    }
    else
    {
      std::vector<Realms::Realm*>::iterator itr;
      for ( itr = gamestate.Realms.begin(); itr != gamestate.Realms.end(); ++itr )
      {
        std::ostringstream thname;
        thname << "Decay_" << ( *itr )->name();
        if ( ( *itr )->is_shadowrealm )
          threadhelp::start_thread( decay_thread_shadow, thname.str().c_str(), (void*)( *itr ) );
        else
          threadhelp::start_thread( decay_thread, thname.str().c_str(), (void*)( *itr ) );
      }
    }
  }
  else
  {
    checkpoint( "don't start decay thread" );
  }

  checkpoint( "start reap thread" );
  threadhelp::start_thread( reap_thread, "Reap" );

  checkpoint( "start dbglisten thread" );
  threadhelp::start_thread( debug_listen_thread, "DbgListn" );

  checkpoint( "start threadstatus thread" );
  start_thread( threadstatus_thread, "ThreadStatus" );

  checkpoint( "start clienttransmit thread" );
  start_thread( Network::ClientTransmitThread, "ClientTransmit" );

#ifdef HAVE_MYSQL
  checkpoint( "start sql service thread" );
  start_sql_service();
#endif
}

#if REFPTR_DEBUG
unsigned int ref_counted::_ctor_calls;
#endif

void display_unreaped_orphan_instances();
void display_reftypes();

void display_leftover_objects()
{
  Bscript::display_executor_instances();
  display_unreaped_orphan_instances();
#if BOBJECTIMP_DEBUG
  Bscript::display_bobjectimp_instances();
#endif
  display_reftypes();
  std::ofstream ofs;
  Clib::OFStreamWriter sw( &ofs );
  sw.init( "leftovers.txt" );
  objStorageManager.objecthash.PrintContents( sw );
  fmt::Writer tmp;
  if ( stateManager.uobjcount.uobject_count != 0 )
    tmp << "Remaining UObjects: " << stateManager.uobjcount.uobject_count << "\n";
  if ( stateManager.uobjcount.ucharacter_count != 0 )
    tmp << "Remaining Mobiles: " << stateManager.uobjcount.ucharacter_count << "\n";
  if ( stateManager.uobjcount.npc_count != 0 )
    tmp << "Remaining NPCs: " << stateManager.uobjcount.npc_count << "\n";
  if ( stateManager.uobjcount.uitem_count != 0 )
    tmp << "Remaining Items: " << stateManager.uobjcount.uitem_count << "\n";
  if ( stateManager.uobjcount.umulti_count != 0 )
    tmp << "Remaining Multis: " << stateManager.uobjcount.umulti_count << "\n";
  if ( stateManager.uobjcount.unreaped_orphans != 0 )
    tmp << "Unreaped orphans: " << stateManager.uobjcount.unreaped_orphans << "\n";
  if ( stateManager.uobjcount.uobj_count_echrref != 0 )
    tmp << "Remaining EChrRef objects: " << stateManager.uobjcount.uobj_count_echrref << "\n";
  if ( Bscript::executor_count )
    tmp << "Remaining Executors: " << Bscript::executor_count << "\n";
  if ( Bscript::eobject_imp_count )
    tmp << "Remaining script objectimps: " << Bscript::eobject_imp_count << "\n";
  INFO_PRINT << tmp.str();
}

void run_start_scripts()
{
  INFO_PRINT << "Running startup script.\n";
  run_script_to_completion( "start" );
  for ( const auto& pkg : Plib::systemstate.packages )
  {
    std::string scriptname = pkg->dir() + "start.ecl";

    if ( Clib::FileExists( scriptname.c_str() ) )
    {
      ScriptDef script( "start", pkg, "" );
      Bscript::BObject obj( run_script_to_completion( script ) );
    }
  }
  INFO_PRINT << "Startup script complete.\n";
}

#ifdef _WIN32
typedef BOOL( WINAPI* DynHeapSetInformation )( PVOID HeapHandle,
                                               HEAP_INFORMATION_CLASS HeapInformationClass,
                                               PVOID HeapInformation,
                                               SIZE_T HeapInformationLength );

const char* Use_low_fragmentation_Heap()
{
  if ( settingsManager.ssopt.use_win_lfh )
  {
    HINSTANCE hKernel32;

    hKernel32 = LoadLibrary( "Kernel32" );
    if ( hKernel32 != nullptr )
    {
      DynHeapSetInformation ProcAdd;
      ProcAdd = (DynHeapSetInformation)GetProcAddress( hKernel32, "HeapSetInformation" );
      if ( ProcAdd != nullptr )
      {
        ULONG HeapFragValue = 2;

        if ( ( ProcAdd )( GetProcessHeap(), HeapCompatibilityInformation, &HeapFragValue,
                          sizeof( HeapFragValue ) ) )
        {
          FreeLibrary( hKernel32 );
          return "low-fragmentation Heap ...activated";
        }
        else
        {
          FreeLibrary( hKernel32 );
          return "low-fragmentation Heap ...not activated";
        }
      }
      else
      {
        FreeLibrary( hKernel32 );
        return "low-fragmentation Heap ...not available on your Windows";
      }
    }
    else
      return "low-fragmentation Heap ...not available on your Windows";
  }
  else
    return "low-fragmentation Heap ...disabled via ServSpecOpt";
}
#endif

#ifdef __linux__
void Check_libc_version()
{
  const char* libc_version = gnu_get_libc_version();

  int main_version = 0;
  int sub_version = 0;
  int build = 0;
  ISTRINGSTREAM is( libc_version );

  if ( is >> main_version )
  {
    char delimiter;
    if ( is >> delimiter >> sub_version )
    {
      is >> delimiter >> build;
    }
  }
  else
    POLLOG_ERROR << "Error in analyzing libc version string [" << libc_version
                 << "]. Please contact Core-Team.\n";

  if ( main_version * 100000000 + sub_version * 10000 + build >= 2 * 100000000 + 3 * 10000 + 2 )
    POLLOG_INFO << "Found libc " << libc_version << " - ok\n";
  else
    POLLOG_ERROR << "Found libc " << libc_version << " - Please update to 2.3.2 or above.\n";
}
#endif


}  // namespace Core

int xmain_inner( bool testing )
{
#ifdef _WIN32
  Clib::MiniDumper::Initialize();
  // Aug. 15, 2006 Austin
  // Added atexit() call to remove the tray icon.
  atexit( Core::ShutdownSystemTrayHandling );
#else
#ifdef __linux__
  std::ofstream polpid;

  polpid.open( ( Plib::systemstate.config.pidfile_path + "pol.pid" ).c_str(),
               std::ios::out | std::ios::trunc );

  if ( polpid.is_open() )
    polpid << Clib::tostring( getpid() );
  else
    INFO_PRINT << "Cannot create pid file in " << Plib::systemstate.config.pidfile_path << "\n";

  polpid.close();
#endif
#endif

  // problem with order of global construction, threads cannot be registered in the constructor of
  // gamestate :(
  Core::gamestate.task_thread_pool.init_pool(
      std::max( 2u, std::thread::hardware_concurrency() / 2 ), "generic_task_thread" );

  int res;

  // for profiling:
  // chdir( "d:\\pol" );
  // PrintAllocationData();

  Clib::MakeDirectory( "log" );

  POLLOG_INFO << POL_VERSION_ID << " - " << Clib::ProgramConfig::build_target() << "\ncompiled on "
              << Clib::ProgramConfig::build_datetime() << "\n"
              << POL_COPYRIGHT << "\n\n";
  if ( testing )
    POLLOG_INFO << "TESTING MODE\n\n";

#ifndef NDEBUG
  POLLOG_INFO << "Sizes: \n"
              << "   UObject:    " << sizeof( Core::UObject ) << "\n"
              << "   Item:       " << sizeof( Items::Item ) << "\n"
              << "   UContainer: " << sizeof( Core::UContainer ) << "\n"
              << "   Character:  " << sizeof( Mobile::Character ) << "\n"
              << "   Client:     " << sizeof( Network::Client ) << "\n"
              << "   NPC:        " << sizeof( Mobile::NPC ) << "\n";

#ifdef __unix__
#ifdef PTHREAD_THREADS_MAX
  POLLOG_INFO << "   Max Threads: " << PTHREAD_THREADS_MAX << "\n";
#endif
#endif
  POLLOG_INFO << "\n";
#endif
  POLLOG_INFO << "Using " << Core::gamestate.task_thread_pool.size() << " out of "
              << std::thread::hardware_concurrency() << " worldsave threads\n";

  Core::checkpoint( "installing signal handlers" );
  Core::install_signal_handlers();

  Core::checkpoint( "starting POL clocks" );
  Core::start_pol_clocks();
  Core::pause_pol_clocks();

  POLLOG_INFO << "Reading Configuration.\n";

  Core::stateManager.gflag_in_system_startup = true;

  Core::checkpoint( "reading pol.cfg" );
  Core::PolConfig::read_pol_config( true );

  Core::checkpoint( "reading config/bannedips.cfg" );
  Network::read_bannedips_config( true );

  Core::checkpoint( "reading servspecopt.cfg" );
  Core::ServSpecOpt::read_servspecopt();

  Core::checkpoint( "reading extobj.cfg" );
  Core::read_extobj();

#ifdef _WIN32
  Core::checkpoint( Core::Use_low_fragmentation_Heap() );
#endif

#ifdef __linux__
  Core::checkpoint( "checking libc version" );
  Core::Check_libc_version();
#endif

  Core::checkpoint( "init default itemdesc defaults" );
  Core::gamestate.empty_itemdesc->doubleclick_range =
      Core::settingsManager.ssopt.default_doubleclick_range;
  Core::gamestate.empty_itemdesc->decay_time = Core::settingsManager.ssopt.default_decay_time;

  Core::checkpoint( "loading POL map file" );
  if ( !Core::load_realms() )
  {
    POLLOG_ERROR << "Unable to load Realms. Please make sure your Realms have been generated by "
                    "UOConvert and your RealmDataPath is set correctly in Pol.cfg.\n";
    return 1;
  }

  // PrintAllocationData();

  Core::checkpoint( "initializing IPC structures" );
  Core::init_ipc_vars();
  threadhelp::init_threadhelp();

#ifdef _WIN32
  Core::InitializeSystemTrayHandling();
#endif

  Core::checkpoint( "initializing sockets library" );
  res = Network::init_sockets_library();
  if ( res < 0 )
  {
    POLLOG_ERROR << "Unable to initialize sockets library.\n";
    return 1;
  }

  Core::checkpoint( "loading configuration" );
  Core::load_data();

  Core::checkpoint( "loading system hooks" );
  Core::load_system_hooks();

  Core::checkpoint( "loading packet hooks" );
  Network::load_packet_hooks();

  Core::checkpoint( "loading auxservice configuration" );
  Network::load_aux_services();

  Core::checkpoint( "reading menus" );
  Core::Menu::read_menus();

  Core::checkpoint( "loading intrinsic weapons" );
  Items::load_intrinsic_weapons();
  Core::checkpoint( "validating intrinsic shield template" );
  Items::validate_intrinsic_shield_template();
  Core::checkpoint( "reading gameservers" );
  Core::read_gameservers();
  Core::checkpoint( "reading starting locations" );
  Core::read_starting_locations();

  if ( testing )
  {
    POLLOG_INFO << "Running POL test suite.\n";
    Testing::run_pol_tests();
    Core::cancel_all_trades();
    Core::stop_gameclock();
    Core::gamestate.deinitialize();
    return 0;
  }

  // PrintAllocationData();
  POLLOG_INFO << "Reading data files:\n";
  {
    Tools::Timer<> timer;
    Core::checkpoint( "reading account data" );
    Accounts::read_account_data();

    Core::checkpoint( "reading data" );
    Core::read_data();
    POLLOG_INFO << "Done! " << timer.ellapsed() << " milliseconds.\n";
  }


  Items::allocate_intrinsic_equipment_serials();
  Core::stateManager.gflag_in_system_startup = false;

  // PrintAllocationData();

  Core::checkpoint( "running start scripts" );
  Core::run_start_scripts();

  Core::checkpoint( "starting client listeners" );
  Core::start_uo_client_listeners();

  POLLOG_INFO << "Initialization complete.  POL is active.  Ctrl-C to stop.\n\n";

  DEINIT_STARTLOG();
  POLLOG.Format( "{0:s} ({1:s}) compiled on {2:s} running.\n" )
      << POL_VERSION_ID << Clib::ProgramConfig::build_target()
      << Clib::ProgramConfig::build_datetime();

  POLLOG_INFO << "Game is active.\n";

  Core::CoreSetSysTrayToolTip( "Running", Core::ToolTipPrioritySystem );

  Core::restart_pol_clocks();
  Core::polclock_checkin();

  // this is done right after reading globals from pol.txt:
  // checkpoint( "starting game clock" );
  // start_gameclock();

  Core::checkpoint( "starting periodic tasks" );
  Core::start_tasks();

  Core::checkpoint( "starting threads" );
  Core::start_threads();
  Network::start_aux_services();

#ifdef _WIN32
  Core::console_thread();
  Core::checkpoint( "exit signal detected" );
  Core::CoreSetSysTrayToolTip( "Shutting down", Core::ToolTipPriorityShutdown );
#else
  // On Linux, signals are directed to a particular thread, if we use pthread_sigmask like we're
  // supposed to.
  // therefore, we have to do this signal checking in this thread.
  threadhelp::start_thread( Core::console_thread, "Console" );

  Core::catch_signals_thread();
#endif
  Core::checkpoint( "waiting for child threads to exit" );
  // NOTE that it's possible that the thread_status thread not have exited yet..
  // it signals the catch_signals_thread (this one) just before it exits.
  // and on windows, we get here right after the console thread exits.
  while ( threadhelp::child_threads )
  {
    Core::pol_sleep_ms( 1000 );
  }
  Core::checkpoint( "child threads have shut down" );

  Core::cancel_all_trades();
  Core::stop_gameclock();
  POLLOG_INFO << "Shutting down...\n";

  Core::checkpoint( "writing data" );
  if ( Core::should_write_data() )
  {
    Core::CoreSetSysTrayToolTip( "Writing data files", Core::ToolTipPriorityShutdown );
    POLLOG_INFO << "Writing data files...";

    Core::PolLock lck;
    unsigned int dirty, clean;
    long long elapsed_ms;
    int savetype;

    if ( Clib::passert_shutdown_due_to_assertion )
      savetype = Plib::systemstate.config.assertion_shutdown_save_type;
    else
      savetype = Plib::systemstate.config.shutdown_save_type;

    // TODO: full save if incremental_saves_disabled ?
    // otherwise could have really, really bad timewarps
    Tools::Timer<> timer;
    if ( savetype == Core::SAVE_FULL )
      Core::write_data( dirty, clean, elapsed_ms );
    else
      Core::save_incremental( dirty, clean, elapsed_ms );
    Core::SaveContext::ready();
    POLLOG_INFO << "Data save completed in " << elapsed_ms << " ms. " << timer.ellapsed()
                << " total.\n";
  }
  else
  {
    if ( Clib::passert_shutdown_due_to_assertion && Clib::passert_nosave )
      POLLOG_INFO << "Not writing data due to assertion failure.\n";
    else if ( Plib::systemstate.config.inhibit_saves )
      POLLOG_INFO << "Not writing data due to pol.cfg InhibitSaves=1 setting.\n";
  }
  Core::gamestate.deinitialize();
  return 0;
}

int xmain_outer( bool testing )
{
  try
  {
    return xmain_inner( testing );
  }
  catch ( std::exception& )
  {
    if ( Core::stateManager.last_checkpoint != nullptr )
    {
      POLLOG_INFO << "Server Shutdown: " << Core::stateManager.last_checkpoint << "\n";
      // pol_sleep_ms( 10000 );
    }
    Core::gamestate.deinitialize();

    throw;
  }
}
}  // namespace Pol
