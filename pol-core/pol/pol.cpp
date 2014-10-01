/*
History
=======
2005/02/23 Shinigami: ServSpecOpt DecayItems to enable/disable item decay
2005/04/03 Shinigami: send_feature_enable() call moved from start_client_char()
to send_start() to send before char selection
2005/04/04 Shinigami: added can_delete_character( chr, delete_by )
2005/06/15 Shinigami: ServSpecOpt UseWinLFH to enable/disable Windows XP/2003 low-fragmentation Heap
added Check_for_WinXP_or_Win2003() and low_fragmentation_Heap()
2005/06/20 Shinigami: added llog (needs defined MEMORYLEAK)
2005/07/01 Shinigami: removed Check_for_WinXP_or_Win2003() and transformed call of
Use_low_fragmentation_Heap() into Run-Time Dynamic Linking
2005/10/13 Shinigami: added Check_libc_version() and printing a Warning if libc is to old
2005/11/25 Shinigami: PKTBI_BF::TYPE_SESPAM will not block Inactivity-Check
2005/11/28 MuadDib:   Created check_inactivity() bool function to handle checking packets
for ones to be considered "ignored" for inactivity. Returns true if
the packet was one to be ignored.
2005/11/28 MuadDib:   Implemented check_inactivity() function in appropriate place.
2006/03/01 MuadDib:   Added connect = true to start_client_char so char creation can use.
2006/03/03 MuadDib:   Moved all instances of connected = true to start_client_char.
2006/06/03 Shinigami: added little bit more logging @ pthread_create
2006/06/05 Shinigami: added little bit more logging @ Client disconnects by Core
2006/07/05 Shinigami: moved MakeDirectory("log") a little bit up
2006/10/07 Shinigami: FreeBSD fix - changed some __linux__ to __unix__
2007/03/08 Shinigami: added pthread_exit and _endhreadex to close threads
2007/05/06 Shinigami: smaller bugfix in Check_libc_version()
2007/06/17 Shinigami: Pergon-Linux-Release generates file "pol.pid"
2007/07/08 Shinigami: added UO:KR login process
2008/07/08 Turley:    removed Checkpoint "initializing random number generator"
2008/12/17 MuadDub:   Added check when loading Realms for no realms existing.
2009/01/19 Nando:     added unload_aux_services() and unload_packages() to the shutdown cleanup
2009/1/24  MuadDib:   Added read_bannedips_config() and checkpoint for it after loading of pol.cfg
2009/07/23 MuadDib:   Updates for MSGOUT naming.
2009/07/31 MuadDib:   xmain_inner(): Force Client Disconnect to initiate cleanup of clients and chars, after shutdown,
before other cleanups.
2009/08/01 MuadDib:   Removed send_tech_stuff(), send_betaclient_BF(), just_ignore_message(), and ignore_69() due to not used or obsolete.
2009/08/03 MuadDib:   Renaming of MSG_HANDLER_6017 and related, to MSG_HANDLER_V2 for better description
Renamed secondary handler class to *_V2 for naming convention
2009/08/14 Turley:    fixed definition of PKTIN_5D
2009/08/19 Turley:    PKTIN_5D clientflag saved in client->UOExpansionFlagClient
2009/09/03 MuadDib:   Relocation of account related cpp/h
Changes for multi related source file relocation
2009/09/15 MuadDib:   Multi registration/unregistration support added.
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
2009/09/22 MuadDib:   Fix for lightlevel resets in client during login.
2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi
2009/12/04 Turley:    Crypto cleanup - Tomi
2010/01/22 Turley:    Speedhack Prevention System
2010/03/28 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within decay_thread_shadow
2011/11/12 Tomi:	  Added extobj.cfg

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include <assert.h>
#ifndef __clang__
#include <omp.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif

#ifdef __unix__
#include <execinfo.h>
#endif

#include <stdio.h>
#include <string.h>

#include "../clib/endian.h"
#include "../clib/esignal.h"
#include "../clib/fdump.h"
#include "../clib/fileutil.h"
#include "../clib/MD5.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"
#include "../clib/strexcpt.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../clib/tracebuf.h"
#include "../clib/timer.h"

#include "../plib/pkg.h"
#include "../plib/realm.h"

#include "accounts/account.h"
#include "allocd.h"
#include "multi/boat.h"
#include "multi/house.h"
#include "network/cgdata.h"
#include "mobile/charactr.h"
#include "checkpnt.h"
#include "network/client.h"
#include "network/cliface.h"
#include "core.h"
#include "decay.h"
#include "extobj.h"
#include "fnsearch.h"
#include "gameclck.h"
#include "gflag.h"
#include "guardrgn.h"
#include "network/iostats.h"
#include "item/itemdesc.h"
#include "lightlvl.h"
#include "loadunld.h"
#include "miscrgn.h"
#include "msghandl.h"
#include "msgfiltr.h"
#include "musicrgn.h"
#include "multi/multi.h"
#include "objecthash.h"
#include "objtype.h"
#include "pktin.h"
#include "pktout.h"
#include "pktboth.h"
#include "realms.h"
#include "network/packethooks.h"
#include "party.h"
#include "polclock.h"
#include "polcfg.h"
#include "poldbg.h"
#include "polfile.h"
#include "polsig.h"
#include "poltest.h"
#include "../plib/polver.h"
#include "polwww.h"
#include "profile.h"
#include "readcfg.h"
#include "savedata.h"
#include "schedule.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "sockets.h"
#include "sockio.h"
#include "ssopt.h"
#include "uobjcnt.h"
#include "uofile.h"
#include "uoscrobj.h"
#include "ufunc.h"
#include "uvars.h"
#include "uworld.h"
#include "crypt/cryptengine.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "polsem.h"
#include "tasks.h"
#include "../bscript/escriptv.h"
#include "containr.h"

#include "stubdata.h"
#include "uimport.h"

#include "../clib/logfacility.h"

#include "sqlscrobj.h"

#ifdef _WIN32
#include "../clib/mdump.h"
#endif

#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#endif

#ifdef __linux__
#include <gnu/libc-version.h>
#endif
namespace Pol {
  namespace Bscript {
    void display_executor_instances();
    void display_bobjectimp_instances();
  }
  namespace Items {
    void unload_itemdesc_scripts();
    void load_intrinsic_weapons();
    void allocate_intrinsic_weapon_serials();
  }
  namespace Network {
    void unload_aux_services(); // added 2009-01-19 (Nando)
    void load_aux_services();
    void start_aux_services();
    void read_bannedips_config( bool initial_load );
  }
  namespace Core {
    void cancel_all_trades();
    void load_system_hooks();
    bool load_realms();
    void clear_listen_points();
    void InitializeSystemTrayHandling();
    void ShutdownSystemTrayHandling();
    void start_uo_client_listeners( void );
    void unload_other_objects();
    void unload_system_hooks();
    void start_tasks();
    void UnloadAllConfigFiles();

    void unload_npc_templates();



    extern void cleanup_vars();

    using namespace threadhelp;

#define CLIENT_CHECKPOINT(x) client->checkpoint = x
    SOCKET listen_socket;
    fd_set listen_fd;
    struct timeval listen_timeout = { 0, 0 };

    fd_set recv_fd;
    fd_set err_fd;
    fd_set send_fd;
    struct timeval select_timeout = { 0, 0 };


    //PacketRegistry pktRegistry;
    MSG_HANDLER handler[256];
    MSG_HANDLER_V2 handler_v2[256];

    MessageHandler::MessageHandler( unsigned char msgtype,
                                    int msglen,
                                    PktHandlerFunc func)
    {
      passert( msglen != 0 );
      /*
      if (handler[msgtype].msglen)
      {
      // DIE
      cerr << "WTF!  Message Handler 0x" << std::hex << msgtype << std::dec << " multiply defined!" << endl;
      exit(1);
      }
      */

      /*
      if (handler[msgtype].msglen != msglen) {
      Log2("Packet %X size has been changed from %d to %d", msgtype, handler[msgtype].msglen, msglen);
      }
      */

      handler[msgtype].func = func;
      handler[msgtype].msglen = msglen;
    }

    MessageHandler_V2::MessageHandler_V2( unsigned char msgtype,
                                          int msglen,
                                          PktHandlerFunc func)
    {
      passert( msglen != 0 );
      /*
      if (handler_v2[msgtype].msglen)
      {
      // DIE
      cerr << "WTF!  Message Handler 0x" << std::hex << msgtype << std::dec << " multiply defined!" << endl;
      exit(1);
      }
      */

      /*
      if (handler_v2[msgtype].msglen != msglen) {
      Log2("Packet %X size has been changed from %d to %d", msgtype, handler_v2[msgtype].msglen, msglen);
      }
      */

      handler_v2[msgtype].func = func;
      handler_v2[msgtype].msglen = msglen;
    }

    void send_startup( Network::Client *client )
    {
      Mobile::Character *chr = client->chr;
      Network::PktHelper::PacketOut<Network::PktOut_1B> msg;
      msg->Write<u32>( chr->serial_ext );
      msg->offset += 4; //u8 unk5, unk6, unk7, unk8
      msg->WriteFlipped<u16>( chr->graphic );
      msg->WriteFlipped<u16>( chr->x );
      msg->WriteFlipped<u16>( chr->y );
      msg->offset++; // u8 unk_15
      msg->Write<s8>( chr->z );
      msg->Write<u8>( chr->facing );
      msg->offset += 3; //u8 unk18,unk19,unk20
      msg->Write<u8>( static_cast<u8>( 0x7F ) );
      msg->offset++; // u8 unk22
      msg->offset += 4; // u16 map_startx, map_starty
      msg->WriteFlipped<u16>( client->chr->realm->width() );
      msg->WriteFlipped<u16>( client->chr->realm->height() );
      msg->offset += 6; // u8 unk31, unk32, unk33, unk34, unk35, unk36
      msg.Send( client );
    }

    bool check_inactivity( Network::Client* client )
    {
      switch ( client->buffer[0] )
      {
        case PKTBI_73_ID:
          // Fallthrough
        case PKTIN_09_ID:
          // Fallthrough
        case PKTBI_D6_IN_ID:
          return true;
        case PKTBI_BF_ID:
          if ( ( client->buffer[3] == 0 ) && ( client->buffer[4] == PKTBI_BF::TYPE_SESPAM ) )
            return true;
          break;
        default:
          return false;
      }

      return false;
    }

    void send_inrange_items( Network::Client* client )
    {
      WorldIterator<ItemFilter>::InVisualRange( client->chr, [&]( Items::Item* item )
      {
        send_item( client, item );
      } );
    }

    void send_inrange_multis( Network::Client* client )
    {
      WorldIterator<MultiFilter>::InVisualRange( client->chr, [&]( Multi::UMulti* multi )
      {
        send_multi( client, multi );
      } );
    }

    void textcmd_startlog( Network::Client* client );
    void textcmd_stoplog( Network::Client* client );
    void start_client_char( Network::Client *client )
    {
      client->ready = 1;
      client->chr->connected = true;

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
        if ( supporting_multi != NULL )
        {
          supporting_multi->register_object( client->chr );
          Multi::UHouse* this_house = supporting_multi->as_house();
          if ( client->chr->registered_house == 0 )
          {
            client->chr->registered_house = supporting_multi->serial;

            if ( this_house != NULL )
              this_house->walk_on( client->chr );
          }
        }
        else
        {
          if ( client->chr->registered_house > 0 )
          {
            Multi::UMulti* multi = system_find_multi( client->chr->registered_house );
            if ( multi != NULL )
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

      if ( ssopt.core_sends_season )
        send_season_info( client );

      client->chr->lastx = client->chr->lasty = client->chr->lastz = 0;

      client->gd->music_region = musicdef->getregion( 0, 0, client->chr->realm );
      client->gd->justice_region = justicedef->getregion( 0, 0, client->chr->realm );
      client->gd->weather_region = weatherdef->getregion( 0, 0, client->chr->realm );

      send_goxyz( client, client->chr );
      client->chr->check_region_changes();

      client->chr->send_warmode();
      login_complete( client );
      client->chr->tellmove();

      client->chr->check_weather_region_change( true );

      if ( ssopt.core_sends_season )
        send_season_info( client );

      send_objects_newly_inrange( client );

      client->chr->send_highlight();
      send_owncreate( client, client->chr );

      send_goxyz( client, client->chr );

      client->restart();

      client->chr->clear_gotten_item();
      on_loggon_party( client->chr );


      //  Moved login_complete higher to prevent weather regions from messing up client 
      //  spell icon packets(made it look like it was raining spell icons from spellbook if logged 
      //  into a weather region with rain.
      //	login_complete(client);
    }


    void call_chr_scripts( Mobile::Character* chr, const string& root_script_ecl, const string& pkg_script_ecl, bool offline = false )
    {
      ScriptDef sd;
      sd.quickconfig( root_script_ecl );

      if ( sd.exists() )
      {
        call_script( sd, offline ? new Module::EOfflineCharacterRefObjImp( chr ) : new Module::ECharacterRefObjImp( chr ) );
      }

      for ( Plib::Packages::iterator itr = Plib::packages.begin(); itr != Plib::packages.end(); ++itr )
      {
        Plib::Package* pkg = *itr;

        sd.quickconfig( pkg, pkg_script_ecl );
        if ( sd.exists() )
        {
          call_script( sd, offline ? new Module::EOfflineCharacterRefObjImp( chr ) : new Module::ECharacterRefObjImp( chr ) );
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
        return call_script( sd, new Module::EOfflineCharacterRefObjImp( chr ), new Bscript::BLong( delete_by ) );
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
    void char_select( Network::Client *client, PKTIN_5D *msg )
    {
      bool reconnecting = false;
      int charidx = cfBEu32( msg->charidx );
      if ( ( charidx >= config.character_slots ) ||
           ( client->acct == NULL ) ||
           ( client->acct->get_character( charidx ) == NULL ) )
      {
        send_login_error( client, LOGIN_ERROR_MISC );
        client->Disconnect();
        return;
      }

      Mobile::Character *chosen_char = client->acct->get_character( charidx );

      POLLOG.Format( "Account {} selecting character {}\n" )
        << client->acct->name()
        << chosen_char->name();

      if ( config.min_cmdlevel_to_login > chosen_char->cmdlevel() )
      {
        POLLOG.Format( "Account {} with character {} doesn't fit MinCmdlevelToLogin from pol.cfg. Client disconnected by Core.\n" )
          << client->acct->name()
          << chosen_char->name();

        send_login_error( client, LOGIN_ERROR_MISC );
        client->Disconnect();
        return;
      }

      //Dave moved this from login.cpp so client cmdlevel can be checked before denying login
      if ( ( ( std::count_if( clients.begin(), clients.end(), clientHasCharacter ) ) >= config.max_clients ) &&
           ( chosen_char->cmdlevel() < config.max_clients_bypass_cmdlevel ) )
      {
        POLLOG.Format( "To much clients connected. Check MaximumClients and/or MaximumClientsBypassCmdLevel in pol.cfg.\nAccount {} with character {} Client disconnected by Core.\n" )
          << client->acct->name()
          << chosen_char->name();

        send_login_error( client, LOGIN_ERROR_MISC );
        client->Disconnect();
        return;
      }

      if ( client->acct->active_character != NULL ) // this account has a currently active character.
      {
        // if it's not the one that was picked, refuse to start this one.
        if ( client->acct->active_character != chosen_char )
        {
          send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
          client->Disconnect();
          return;
        }

        // we're reattaching to a character that is in-game.  If there is still
        // a client attached, disconnect it.
        if ( chosen_char->client )
        {
          chosen_char->client->gd->clear();
          chosen_char->client->forceDisconnect();
          chosen_char->client->ready = 0;
          chosen_char->client->msgtype_filter = &disconnected_filter;


          // disassociate the objects from each other.
          chosen_char->client->acct = NULL;
          chosen_char->client->chr = NULL;

          chosen_char->client = NULL;
        }
        reconnecting = true;
      }
      else
      {
        // logging in a character that's offline.
        SetCharacterWorldPosition( chosen_char );
        chosen_char->logged_in = true;
      }

      client->acct->active_character = chosen_char;
      client->chr = chosen_char;
      chosen_char->client = client;
      chosen_char->acct.set( client->acct );

      client->UOExpansionFlagClient = cfBEu32( msg->clientflags );

      client->msgtype_filter = &game_filter;
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
    MESSAGE_HANDLER( PKTIN_5D, char_select );

    void send_client_char_data( Mobile::Character *chr, Network::Client *client );
    void handle_resync_request( Network::Client* client, PKTBI_22_SYNC* msg )
    {
      send_goxyz( client, client->chr );

      client->send_pause(); //dave removed force=true 5/10/3, let uoclient.cfg option determine xflow packets (else this hangs 4.0.0e clients)

      Core::WorldIterator<Core::MobileFilter>::InVisualRange( client->chr, [&]( Mobile::Character* zonechr ) { send_client_char_data( zonechr, client ); } );

      send_inrange_items( client );
      send_inrange_multis( client );

      client->send_restart();//dave removed force=true 5/10/3
    }
    MESSAGE_HANDLER( PKTBI_22_SYNC, handle_resync_request );

    void handle_keep_alive( Network::Client *client, PKTBI_73 *msg )
    {
      transmit( client, msg, sizeof *msg );
    }
    MESSAGE_HANDLER( PKTBI_73, handle_keep_alive );

    void handle_unknown_packet( Network::Client* client )
    {
      if ( config.display_unknown_packets )
      {
        fmt::Writer tmp;
        tmp.Format( "Unknown packet type 0x{:X}: {} bytes (IP:{}, Account:{})\n" )
          << (int)client->buffer[0] << client->bytes_received
          << client->ipaddrAsString() << ( ( client->acct != NULL ) ? client->acct->name() : "None" );

        if ( client->bytes_received <= 64 )
        {
          Clib::fdump( tmp, &client->buffer, client->bytes_received );
          POLLOG_INFO << tmp.c_str() << "\n";
        }
        else
        {
          INFO_PRINT << tmp.c_str() << "\n";
          Clib::fdump( tmp, &client->buffer, client->bytes_received );
          POLLOG << tmp.c_str() << "\n";
        }
      }
    }

    void restart_all_clients()
    {
      if ( !uoclient_protocol.EnableFlowControlPackets )
        return;
      for ( Clients::iterator itr = clients.begin(), end = clients.end();
            itr != end;
            ++itr )
      {
        Network::Client* client = ( *itr );
        if ( client->pause_count )
        {
          client->restart();
        }
      }
    }

    // bool - return true when a message was processed.
    bool process_data( Network::Client *client )
    {
      // NOTE: This is coded such that for normal messages, which are completely available,
      // this function will get the type, then the length, then the data, without having
      // to wait for a second or third call.
      // Also, the abnormal state, RECV_STATE_CRYPTSEED_WAIT, is handled at the end, so in 
      // normal processing its code doesn't see the code path.
      passert( client->bufcheck1_AA == 0xAA );
      passert( client->bufcheck2_55 == 0x55 );
      if ( client->recv_state == Network::Client::RECV_STATE_MSGTYPE_WAIT )
      {
        client->bytes_received = 0;
        client->recv_remaining( 1 );
        CLIENT_CHECKPOINT( 22 );
        if ( client->bytes_received < 1 ) // this really should never happen.
        {
          client->forceDisconnect();
          return false;
        }

        unsigned char msgtype = client->buffer[0];
        client->last_msgtype = msgtype; //CNXBUG
        if ( config.verbose )
          INFO_PRINT.Format( "Incoming msg type: 0x{:X}\n" ) << (int)msgtype;

        if ( ( !handler[msgtype].msglen ) && ( !handler_v2[msgtype].msglen ) )
        {
          INFO_PRINT.Format( "Undefined message type 0x{:X}\n" ) << (int)msgtype;
          client->recv_remaining( sizeof client->buffer / 2 );
          fmt::Writer tmp;
          tmp.Format( "Client#{}: Unexpected message type 0x{:X}, {} bytes( IP : {}, Account : {} )\n" )
            << client->instance_ << (int)msgtype << client->bytes_received
            << client->ipaddrAsString() << ( ( client->acct != NULL ) ? client->acct->name() : "None" );
          if ( client->bytes_received <= 64 )
          {
            Clib::fdump( tmp, client->buffer, client->bytes_received );
            POLLOG_INFO << tmp.c_str() << "\n";
          }
          else
          {
            INFO_PRINT << tmp.c_str();
            Clib::fdump( tmp, client->buffer, client->bytes_received );
            POLLOG << tmp.c_str() << "\n";
          }

          // remain in RECV_STATE_MSGTYPE_WAIT

          return false;
        }
        // It's in the 6017 handlers
        if ( ( client->ClientType & Network::CLIENTTYPE_6017 ) && ( handler_v2[msgtype].msglen ) )
        {
          if ( handler_v2[msgtype].msglen == MSGLEN_2BYTELEN_DATA )
          {
            client->recv_state = Network::Client::RECV_STATE_MSGLEN_WAIT;
          }
          else
          {
            // (handler_v2[msgtype].msglen > 0) must be true
            client->recv_state = Network::Client::RECV_STATE_MSGDATA_WAIT;
            client->message_length = handler_v2[msgtype].msglen;
          }
        }
        else if ( handler[msgtype].msglen )
        {
          if ( handler[msgtype].msglen == MSGLEN_2BYTELEN_DATA )
          {
            client->recv_state = Network::Client::RECV_STATE_MSGLEN_WAIT;
          }
          else
          {
            // (handler[msgtype].msglen > 0) must be true
            client->recv_state = Network::Client::RECV_STATE_MSGDATA_WAIT;
            client->message_length = handler[msgtype].msglen;
          }
        }
      }

      if ( client->recv_state == Network::Client::RECV_STATE_MSGLEN_WAIT )
      {
        client->recv_remaining( 3 );
        CLIENT_CHECKPOINT( 23 );
        if ( client->bytes_received == 3 ) // the length bytes were available.
        {
          // MSG is [MSGTYPE] [LENHI] [LENLO] [DATA ... ]
          client->message_length = ( client->buffer[1] << 8 ) + client->buffer[2];
          if ( client->message_length > sizeof client->buffer )
          {
            POLLOG_INFO.Format( "Client#{}: Too-long message type 0x{:X} length {}\n" )
              << client->instance_
              << (int)client->buffer[0]
              << client->message_length;
            client->message_length = sizeof client->buffer;
          }
          else if ( client->message_length < 3 )
          {
            POLLOG_INFO.Format( "Client#{}: Too-short message length of {}\n" )
              << client->instance_
              << client->message_length;
            client->message_length = 3;
          }
          client->recv_state = Network::Client::RECV_STATE_MSGDATA_WAIT;
        }
        // else keep waiting. 
      }

      if ( client->recv_state == Network::Client::RECV_STATE_MSGDATA_WAIT )
      {
        CLIENT_CHECKPOINT( 24 );
        client->recv_remaining( client->message_length );
        CLIENT_CHECKPOINT( 25 );
        if ( client->bytes_received == client->message_length ) // we have the whole message
        {
          unsigned char msgtype = client->buffer[0];
          Network::iostats.received[msgtype].count++;
          Network::iostats.received[msgtype].bytes += client->message_length;
          if ( !client->fpLog.empty() )
          {
            fmt::Writer tmp;
            tmp << "Client -> Server: 0x" << fmt::hexu( msgtype ) << ", " << client->message_length << " bytes\n";
            Clib::fdump( tmp, &client->buffer, client->message_length );
            FLEXLOG( client->fpLog ) << tmp.c_str() << "\n";
          }

          if ( config.verbose )
            INFO_PRINT.Format( "Message Received: Type 0x{:X}, Length {} bytes\n" ) << (int)msgtype << client->message_length;

          PolLock lck; //multithread
          // it can happen that a client gets disconnected while waiting for the lock.
          if ( client->isConnected() )
          {
            if ( client->msgtype_filter->msgtype_allowed[msgtype] )
            {
              //region Speedhack
              if ( ( ssopt.speedhack_prevention ) && ( msgtype == PKTIN_02_ID ) )
              {
                if ( !client->SpeedHackPrevention() )
                {
                  // client->SpeedHackPrevention() added packet to queue
                  client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
                  CLIENT_CHECKPOINT( 28 );
                  return true;
                }
              }
              //endregion Speedhack
              if ( ( client->ClientType & Network::CLIENTTYPE_6017 ) && ( handler_v2[msgtype].msglen ) )
              {
                try
                {
                  INFO_PRINT_TRACE( 10 ) << "Client#" << client->instance_ << ": message 0x" << fmt::hexu( msgtype ) << "\n";
                  CLIENT_CHECKPOINT( 26 );
                  ( *handler_v2[msgtype].func )( client, client->buffer );
                  CLIENT_CHECKPOINT( 27 );
                  restart_all_clients();
                }
                catch ( std::exception& ex )
                {
                  POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                    << client->instance_
                    << (int)msgtype
                    << ex.what();
                  fmt::Writer tmp;
                  Clib::fdump( tmp, client->buffer, client->bytes_received );
                  POLLOG << tmp.c_str() << "\n";
                  restart_all_clients();
                  throw;
                }
              }
              else // else this is the legacy style (pre-uokr)
              {
                try
                {
                  INFO_PRINT_TRACE( 10 ) << "Client#" << client->instance_ << ": message 0x" << fmt::hexu( msgtype ) << "\n";
                  CLIENT_CHECKPOINT( 26 );
                  ( *handler[msgtype].func )( client, client->buffer );
                  CLIENT_CHECKPOINT( 27 );
                  restart_all_clients();
                }
                catch ( std::exception& ex )
                {
                  POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                    << client->instance_
                    << (int)msgtype
                    << ex.what();
                  fmt::Writer tmp;
                  Clib::fdump( tmp, client->buffer, client->bytes_received );
                  POLLOG << tmp.c_str() << "\n";
                  restart_all_clients();
                  throw;
                }
              }
            }
            else
            {
              POLLOG_ERROR.Format( "Client#{} ({}, Acct {}) sent non-allowed message type 0x{:X}.\n" )
                << client->instance_
                << Network::AddressToString( &client->ipaddr )
                << ( client->acct ? client->acct->name() : "unknown" )
                << (int)msgtype;
            }
          }
          client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
          CLIENT_CHECKPOINT( 28 );
          return true;
        }
        // else keep waiting 
      }
      else if ( client->recv_state == Network::Client::RECV_STATE_CRYPTSEED_WAIT )
      {	   // The abnormal case.  
        // The first four bytes after connection are the 
        // crypto seed
        client->recv_remaining_nocrypt( 4 );

        if ( client->bytes_received == 4 )
        {
          /* The first four bytes transmitted are the encryption seed */
          unsigned char cstype = client->buffer[0];

          if ( ( client->buffer[0] == 0xff ) && ( client->buffer[1] == 0xff ) && ( client->buffer[2] == 0xff ) && ( client->buffer[3] == 0xff ) )
          {
            if ( config.verbose )
            {
              INFO_PRINT.Format( "UOKR Seed Message Received: Type 0x{:X}\n" ) << (int)cstype;
            }
            Network::PktHelper::PacketOut<Network::PktOut_E3> msg;
            msg->WriteFlipped<u16>( static_cast<u16>( 77 ) );
            msg->WriteFlipped<u32>( static_cast<u32>( 0x03 ) );
            msg->Write<u8>( static_cast<u8>( 0x02 ) );	msg->Write<u8>( static_cast<u8>( 0x01 ) );	msg->Write<u8>( static_cast<u8>( 0x03 ) );
            msg->WriteFlipped<u32>( static_cast<u32>( 0x13 ) );
            msg->Write<u8>( static_cast<u8>( 0x02 ) );	msg->Write<u8>( static_cast<u8>( 0x11 ) );	msg->Write<u8>( static_cast<u8>( 0x00 ) );
            msg->Write<u8>( static_cast<u8>( 0xfc ) );	msg->Write<u8>( static_cast<u8>( 0x2f ) );	msg->Write<u8>( static_cast<u8>( 0xe3 ) );
            msg->Write<u8>( static_cast<u8>( 0x81 ) );	msg->Write<u8>( static_cast<u8>( 0x93 ) );	msg->Write<u8>( static_cast<u8>( 0xcb ) );
            msg->Write<u8>( static_cast<u8>( 0xaf ) );	msg->Write<u8>( static_cast<u8>( 0x98 ) );	msg->Write<u8>( static_cast<u8>( 0xdd ) );
            msg->Write<u8>( static_cast<u8>( 0x83 ) );	msg->Write<u8>( static_cast<u8>( 0x13 ) );	msg->Write<u8>( static_cast<u8>( 0xd2 ) );
            msg->Write<u8>( static_cast<u8>( 0x9e ) );	msg->Write<u8>( static_cast<u8>( 0xea ) );	msg->Write<u8>( static_cast<u8>( 0xe4 ) );
            msg->Write<u8>( static_cast<u8>( 0x13 ) );
            msg->WriteFlipped<u32>( static_cast<u32>( 0x10 ) );
            msg->Write<u8>( static_cast<u8>( 0x78 ) );	msg->Write<u8>( static_cast<u8>( 0x13 ) );	msg->Write<u8>( static_cast<u8>( 0xb7 ) );
            msg->Write<u8>( static_cast<u8>( 0x7b ) );	msg->Write<u8>( static_cast<u8>( 0xce ) );	msg->Write<u8>( static_cast<u8>( 0xa8 ) );
            msg->Write<u8>( static_cast<u8>( 0xd7 ) );	msg->Write<u8>( static_cast<u8>( 0xbc ) );	msg->Write<u8>( static_cast<u8>( 0x52 ) );
            msg->Write<u8>( static_cast<u8>( 0xde ) );	msg->Write<u8>( static_cast<u8>( 0x38 ) );	msg->Write<u8>( static_cast<u8>( 0x30 ) );
            msg->Write<u8>( static_cast<u8>( 0xea ) );	msg->Write<u8>( static_cast<u8>( 0xe9 ) );	msg->Write<u8>( static_cast<u8>( 0x1e ) );
            msg->Write<u8>( static_cast<u8>( 0xa3 ) );
            msg->WriteFlipped<u32>( static_cast<u32>( 0x20 ) );
            msg->WriteFlipped<u32>( static_cast<u32>( 0x10 ) );
            msg->Write<u8>( static_cast<u8>( 0x5a ) );	msg->Write<u8>( static_cast<u8>( 0xce ) );	msg->Write<u8>( static_cast<u8>( 0x3e ) );
            msg->Write<u8>( static_cast<u8>( 0xe3 ) );	msg->Write<u8>( static_cast<u8>( 0x97 ) );	msg->Write<u8>( static_cast<u8>( 0x92 ) );
            msg->Write<u8>( static_cast<u8>( 0xe4 ) );	msg->Write<u8>( static_cast<u8>( 0x8a ) );	msg->Write<u8>( static_cast<u8>( 0xf1 ) );
            msg->Write<u8>( static_cast<u8>( 0x9a ) );	msg->Write<u8>( static_cast<u8>( 0xd3 ) );	msg->Write<u8>( static_cast<u8>( 0x04 ) );
            msg->Write<u8>( static_cast<u8>( 0x41 ) );	msg->Write<u8>( static_cast<u8>( 0x03 ) );	msg->Write<u8>( static_cast<u8>( 0xcb ) );
            msg->Write<u8>( static_cast<u8>( 0x53 ) );
            client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
            client->setClientType( Network::CLIENTTYPE_UOKR ); // UO:KR logging in				
            msg.Send( client );
          }
          else if ( client->buffer[0] == PKTIN_EF_ID )  // new seed since 6.0.5.0 (0xef should never appear in normal ipseed)
          {
            if ( config.verbose )
            {
              INFO_PRINT.Format( "6.0.5.0+ Crypt Seed Message Received: Type 0x{:X}\n" ) << (int)cstype;
            }
            client->recv_state = Network::Client::RECV_STATE_CLIENTVERSION_WAIT;
          }
          else
          {
            client->cryptengine->Init( client->buffer, Crypt::CCryptBase::typeAuto );
            client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
          }
        }
        // Else keep waiting for IP address. 
      }
      if ( client->recv_state == Network::Client::RECV_STATE_CLIENTVERSION_WAIT )
      {
        client->recv_remaining_nocrypt( 21 ); // receive and send to handler to get directly the version
        if ( client->bytes_received == 21 )
        {
          client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
          unsigned char tempseed[4];
          tempseed[0] = client->buffer[1];
          tempseed[1] = client->buffer[2];
          tempseed[2] = client->buffer[3];
          tempseed[3] = client->buffer[4];
          client->cryptengine->Init( tempseed, Crypt::CCryptBase::typeLogin );
          ( *handler[PKTIN_EF_ID].func )( client, client->buffer );
        }
      }

      return false;
    }


    void kill_disconnected_clients( void )
    {
      Clients::iterator itr = clients.begin();
      while ( itr != clients.end() )
      {
        Network::Client* client = *itr;
        if ( !client->isReallyConnected() )
        {
          fmt::Writer tmp;
          tmp.Format( "Disconnecting Client#{} ({}/{})" )
            << client->instance_
            << ( client->acct ? client->acct->name() : "[no account]" )
            << ( client->chr ? client->chr->name() : "[no character]" );
          ERROR_PRINT << tmp.c_str() << "\n";
          if ( config.loglevel >= 4 )
            POLLOG << tmp.c_str() << "\n";

          Network::Client::Delete( client );
          client = NULL;
          itr = clients.erase( itr );
        }
        else
        {
          ++itr;
        }
      }
    }

    polclock_t checkin_clock_times_out_at;

    void polclock_checkin()
    {
      checkin_clock_times_out_at = polclock() + 30 * POLCLOCKS_PER_SEC;
    }

    bool client_io_thread( Network::Client* client, bool login )
    {
      fd_set c_recv_fd;
      fd_set c_err_fd;
      fd_set c_send_fd;
      struct timeval c_select_timeout = { 0, 0 };
      int checkpoint = 0;
      int nidle = 0;
      polclock_t last_activity;
      polclock_t last_packet_at = polclock();
      if ( !login )
      {
        if ( config.loglevel >= 11 )
        {
          POLLOG.Format( "Network::Client#{} i/o thread starting\n" ) << client->instance_;
        }
      }
      client->checkpoint = 60; //CNXBUG
      {
        PolLock lck;
        client->checkpoint = 61; //CNXBUG
        last_activity = polclock();
      }
      if ( !login )
      {
        if ( config.loglevel >= 11 )
        {
          POLLOG.Format( "Client#{} i/o thread past initial lock\n" ) << client->instance_;
        }
      }
      CLIENT_CHECKPOINT( 0 );
      try
      {
        while ( !Clib::exit_signalled && client->isReallyConnected() )
        {
          CLIENT_CHECKPOINT( 1 );
          SOCKET nfds = 0;
          FD_ZERO( &c_recv_fd );
          FD_ZERO( &c_err_fd );
          FD_ZERO( &c_send_fd );
          checkpoint = 1;
          FD_SET( client->csocket, &c_recv_fd );
          FD_SET( client->csocket, &c_err_fd );
          if ( client->have_queued_data() )
            FD_SET( client->csocket, &c_send_fd );
          checkpoint = 2;
          if ( (SOCKET)( client->csocket + 1 ) > nfds )
            nfds = client->csocket + 1;

          int res;
          do
          {
            if ( login )
            {
              c_select_timeout.tv_sec = 0;
              c_select_timeout.tv_usec = config.select_timeout_usecs;
            }
            else
            {
              c_select_timeout.tv_sec = 2;
              c_select_timeout.tv_usec = 0;
            }
            CLIENT_CHECKPOINT( 2 );
            res = select( static_cast<int>( nfds ), &c_recv_fd, &c_send_fd, &c_err_fd, &c_select_timeout );
            CLIENT_CHECKPOINT( 3 );
          } while ( res < 0 && !Clib::exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );
          checkpoint = 3;

          if ( res < 0 )
          {
            int sckerr = socket_errno;
            POLLOG.Format( "Client#{}: select res={}, sckerr={}\n" ) << client->instance_ << res << sckerr;
            break;
          }
          else if ( res == 0 )
          {
            if ( ( !client->chr || client->chr->cmdlevel() < config.min_cmdlvl_ignore_inactivity ) &&
                 config.inactivity_warning_timeout && config.inactivity_disconnect_timeout )
            {
              ++nidle;
              if ( nidle == 30 * config.inactivity_warning_timeout )
              {
                CLIENT_CHECKPOINT( 4 );
                PolLock lck; //multithread
                Network::PktHelper::PacketOut<Network::PktOut_53> msg;
                msg->Write<u8>( static_cast<u8>( PKTOUT_53_WARN_CHARACTER_IDLE ) );
                CLIENT_CHECKPOINT( 5 );
                msg.Send( client );
                CLIENT_CHECKPOINT( 18 );
                if ( client->pause_count )
                  client->restart();
              }
              else if ( nidle == 30 * config.inactivity_disconnect_timeout )
              {
                client->forceDisconnect();
              }
            }
          }

          CLIENT_CHECKPOINT( 19 );
          if ( !client->isReallyConnected() )
            break;

          if ( FD_ISSET( client->csocket, &c_err_fd ) )
          {
            client->forceDisconnect();
            break;
          }

          //region Speedhack
          if ( !client->movementqueue.empty() ) // not empty then process the first packet
          {
            PolLock lck; //multithread
            Network::PacketThrottler pkt = client->movementqueue.front();
            if ( client->SpeedHackPrevention( false ) )
            {
              if ( client->isReallyConnected() )
              {
                unsigned char msgtype = pkt.pktbuffer[0];
                if ( ( client->ClientType & Network::CLIENTTYPE_6017 ) && ( handler_v2[msgtype].msglen ) )
                {
                  try
                  {
                    INFO_PRINT_TRACE( 10 ) << "Client#" << client->instance_ << ": message 0x" << fmt::hexu( msgtype ) << "\n";

                    CLIENT_CHECKPOINT( 26 );
                    ( *handler_v2[msgtype].func )( client, pkt.pktbuffer );
                    CLIENT_CHECKPOINT( 27 );
                    restart_all_clients();
                  }
                  catch ( std::exception& ex )
                  {
                    POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                      << client->instance_
                      << (int)msgtype
                      << ex.what();
                    fmt::Writer tmp;
                    Clib::fdump( tmp, pkt.pktbuffer, 7 );
                    POLLOG << tmp.c_str() << "\n";
                    restart_all_clients();
                    throw;
                  }
                }
                else // else this is the legacy style (pre-uokr)
                {
                  try
                  {
                    INFO_PRINT_TRACE( 10 ) << "Client#" << client->instance_ << ": message 0x" << fmt::hexu( msgtype ) << "\n";
                    CLIENT_CHECKPOINT( 26 );
                    ( *handler[msgtype].func )( client, pkt.pktbuffer );
                    CLIENT_CHECKPOINT( 27 );
                    restart_all_clients();
                  }
                  catch ( std::exception& ex )
                  {
                    POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                      << client->instance_
                      << (int)msgtype
                      << ex.what();
                    fmt::Writer tmp;
                    Clib::fdump( tmp, pkt.pktbuffer, 7 );
                    POLLOG << tmp.c_str() << "\n";
                    restart_all_clients();
                    throw;
                  }
                }
              }
              client->movementqueue.pop();
            }
          }
          //endregion Speedhack


          if ( FD_ISSET( client->csocket, &c_recv_fd ) )
          {
            checkpoint = 4;
            CLIENT_CHECKPOINT( 6 );
            if ( process_data( client ) )
            {
              CLIENT_CHECKPOINT( 17 );
              PolLock lck;

              //reset packet timer
              last_packet_at = polclock();
              if ( !check_inactivity( client ) )
              {
                nidle = 0;
                last_activity = polclock();
              }

              checkpoint = 5;
              CLIENT_CHECKPOINT( 7 );
              send_pulse();
              if ( TaskScheduler::is_dirty() )
                wake_tasks_thread();
            }

          }
          checkpoint = 6;

          polclock_t polclock_now = polclock();
          if ( ( polclock_now - last_packet_at ) >= 120000 ) //2 mins
          {
            client->forceDisconnect();
            break;
          }

          if ( client->have_queued_data() && FD_ISSET( client->csocket, &c_send_fd ) )
          {
            PolLock lck;
            CLIENT_CHECKPOINT( 8 );
            client->send_queued_data();
          }
          checkpoint = 7;
          CLIENT_CHECKPOINT( 21 );
          if ( login )
            break;
        }
      }
      catch ( string& str )
      {
        POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
          << client->instance_ << str << checkpoint;
      }
      catch ( const char* msg )
      {
        POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
          << client->instance_ << msg << checkpoint;
      }
      catch ( exception& ex )
      {
        POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
          << client->instance_ << ex.what() << checkpoint;
      }
      CLIENT_CHECKPOINT( 20 );

      if ( login && client->isConnected() )
        return true;
      POLLOG.Format( "Client#{} ({}): disconnected (account {})\n" )
        << client->instance_
        << Network::AddressToString( &client->ipaddr )
        << ( ( client->acct != NULL ) ? client->acct->name() : "unknown" );


      try
      {
        //		if (1)
        {
          CLIENT_CHECKPOINT( 9 );
          PolLock lck;
          clients.erase( std::find( clients.begin(), clients.end(), client ) );
          std::lock_guard<std::mutex> lock( client->_SocketMutex );
          client->closeConnection();
          INFO_PRINT << "Client disconnected from " << Network::AddressToString( &client->ipaddr )
            << " (" << clients.size() << " connections)\n";

          CoreSetSysTrayToolTip( Clib::tostring( clients.size() ) + " clients connected", ToolTipPrioritySystem );
        }

        checkpoint = 8;
        CLIENT_CHECKPOINT( 10 );
        if ( client->chr )
        {
          //			if (1)
          int seconds_wait = 0;
          {
            CLIENT_CHECKPOINT( 11 );
            PolLock lck;

            client->chr->disconnect_cleanup();
            client->gd->clear();
            client->chr->connected = false;
            ScriptDef sd;
            sd.quickconfig( "scripts/misc/logofftest.ecl" );
            if ( sd.exists() )
            {
              CLIENT_CHECKPOINT( 12 );
              Bscript::BObject bobj( run_script_to_completion( sd, new Module::ECharacterRefObjImp( client->chr ) ) );
              if ( bobj.isa( Bscript::BObjectImp::OTLong ) )
              {
                const Bscript::BLong* blong = static_cast<const Bscript::BLong*>( bobj.impptr() );
                seconds_wait = blong->value();
              }
            }
          }

          polclock_t when_logoff = last_activity + seconds_wait * POLCLOCKS_PER_SEC;

          checkpoint = 9;
          CLIENT_CHECKPOINT( 13 );
          while ( !Clib::exit_signalled )
          {
            CLIENT_CHECKPOINT( 14 );
            {
              PolLock lck;
              if ( polclock() >= when_logoff )
                break;
            }
            pol_sleep_ms( 2000 );
          }

          checkpoint = 10;
          CLIENT_CHECKPOINT( 15 );
          //			if (1)
          {
            PolLock lck;
            if ( client->chr )
            {
              Mobile::Character* chr = client->chr;
              CLIENT_CHECKPOINT( 16 );
              call_chr_scripts( chr, "scripts/misc/logoff.ecl", "logoff.ecl" );
              WorldIterator<NPCFilter>::InRange( chr->x, chr->y, chr->realm, 32, [&]( Mobile::Character* zonechr ) { Mobile::NpcPropagateLeftArea( zonechr, chr ); } );
            }
          }
        }
      }
      catch ( std::exception& ex )
      {
        POLLOG.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={}, what={})\n" )
          << client->instance_ << checkpoint << ex.what();
      }

      //if (1)
      {
        PolLock lck;
        CLIENT_CHECKPOINT( 17 );
        Network::Client::Delete( client );
        client = NULL;
      }
      return false;
    }





#define clock_t_to_ms(x) (x)

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
          tasks_thread_sleep( polticks_t_to_ms( sleeptime ) );
          THREAD_CHECKPOINT( tasks, 9 );
        }
      }
      catch ( const char* msg )
      {
        POLLOG.Format( "Tasks Thread exits due to exception: {}\n" ) << msg;
        throw;
      }
      catch ( string& str )
      {
        POLLOG.Format( "Tasks Thread exits due to exception: {}\n" ) << str;
        throw;
      }
      catch ( exception& ex )
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
          ++script_passes;
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
          ++script_passes_activity;
        }
        else
        {
          ++script_passes_noactivity;
        }

        if ( sleeptime )
        {
          THREAD_CHECKPOINT( scripts, 54 );

          wait_for_pulse( polticks_t_to_ms( sleeptime ) );

          THREAD_CHECKPOINT( scripts, 55 );
        }
      }
    }

    void combined_thread( void )
    {
      polclock_t sleeptime;
      bool activity;
      polclock_t script_clocksleft, scheduler_clocksleft;
      polclock_t sleep_clocks;
      polclock_t now;
      while ( !Clib::exit_signalled )
      {
        ++script_passes;
        do
        {
          PolLock lck;
          step_scripts( &sleeptime, &activity );
          check_scheduled_tasks( &sleeptime, &activity );
          restart_all_clients();
          now = polclock();
          script_clocksleft = calc_script_clocksleft( now );
          scheduler_clocksleft = calc_scheduler_clocksleft( now );
          if ( script_clocksleft < scheduler_clocksleft )
            sleep_clocks = script_clocksleft;
          else
            sleep_clocks = scheduler_clocksleft;
        } while ( sleep_clocks <= 0 );

        wait_for_pulse( clock_t_to_ms( sleep_clocks ) );
      }
    }

    void decay_thread( void* );
    void decay_thread_shadow( void* );

    template<class T>
    inline void Delete( T* p )
    {
      delete p;
    }

    template<class T>
    class delete_ob
    {
    public:
      void operator()( T* p )
      {
        delete p;
      }
    };

    void reap_thread( void )
    {
      while ( !Clib::exit_signalled )
      {
        {
          PolLock lck;
          polclock_checkin();
          objecthash.Reap();
          for ( auto &item : Items::dynamic_item_descriptors )
          {
            delete item;
          }
          Items::dynamic_item_descriptors.clear();
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
        if ( !polclock_paused_at )
        {
          polclock_t now = polclock();
          if ( now >= checkin_clock_times_out_at )
          {
            ERROR_PRINT << "No clock movement in 30 seconds.  Dumping thread status.\n";
            report_status_signalled = true;
            checkin_clock_times_out_at = now + 30 * POLCLOCKS_PER_SEC;
          }
        }

        if ( report_status_signalled )
        {
          fmt::Writer tmp;
          tmp << "*Thread Info*\n";
          tmp << "Semaphore PID: " << locker << "\n";
#ifdef __unix__
          if ( locker )
          {
            tmp << "  (\"kill -SIGUSR2 " << locker << "\" to output backtrace)\n";
          }
          void* bt[200];
          char **strings;
          int n = backtrace( bt, 200 );
          strings = backtrace_symbols( bt, n );
          for ( int j = 0; j < n; j++ )
            tmp << strings[j] << "\n";

          free( strings );
#endif
          tmp << "Scripts Thread Checkpoint: " << scripts_thread_checkpoint << "\n";
          tmp << "Last Script: " << Clib::scripts_thread_script << " PC: " << Clib::scripts_thread_scriptPC << "\n";
          tmp << "Escript Instruction Cycles: " << Bscript::escript_instr_cycles << "\n";
          tmp << "Tasks Thread Checkpoint: " << tasks_thread_checkpoint << "\n";
          tmp << "Active Client Thread Checkpoint: " << active_client_thread_checkpoint << "\n";
          if ( check_attack_after_move_function_checkpoint )
            tmp << "check_attack_after_move() Checkpoint: " << check_attack_after_move_function_checkpoint << "\n";
          tmp << "Current Threads:" << "\n";
          ThreadMap::Contents contents;
          threadmap.CopyContents( contents );
          for ( ThreadMap::Contents::const_iterator citr = contents.begin(); citr != contents.end(); ++citr )
          {
            tmp << ( *citr ).first << " - " << ( *citr ).second << "\n";
          }
          tmp << "Child threads (child_threads): " << threadhelp::child_threads << "\n";
          tmp << "Registered threads (ThreadMap): " << contents.size() << "\n";
          report_status_signalled = false;
          ERROR_PRINT << tmp.c_str();
        }
        if ( Clib::exit_signalled )
        {
          if ( !sent_wakeups )
          {
            send_pulse();
            wake_tasks_thread();
            Network::ClientTransmitSingleton::get().Cancel();
#ifdef HAVE_MYSQL
            sql_service.stop();
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
      //cerr << "threadstatus thread exits." << endl;
      signal_catch_thread();
    }

    void catch_signals_thread( void );
    void check_console_commands();
    void reload_configuration();

    void console_thread( void )
    {
      while ( !Clib::exit_signalled )
      {
        pol_sleep_ms( 1000 );

        check_console_commands();
#ifndef _WIN32
        if ( reload_configuration_signalled )
        {
          PolLock lck;
          INFO_PRINT << "Reloading configuration...";
          reload_configuration_signalled = false;
          reload_configuration( );
          INFO_PRINT << "Done.\n";
        }
#endif
      }
    }

    void start_threads()
    {
      threadmap.Register( thread_pid(), "Main" );

      if ( config.web_server )
        start_http_server();

      if ( config.multithread == 1 )
      {
        checkpoint( "start tasks thread" );
        threadhelp::start_thread( tasks_thread, "Tasks" );
        checkpoint( "start scripts thread" );
        threadhelp::start_thread( scripts_thread, "Scripts" );
      }
      else
      {
        checkpoint( "start combined scripts/tasks thread" );
        threadhelp::start_thread( combined_thread, "Combined" );
      }

      if ( ssopt.decay_items )
      {
        checkpoint( "start decay thread" );
        vector<Plib::Realm*>::iterator itr;
        for ( itr = Realms->begin(); itr != Realms->end(); ++itr )
        {
          ostringstream thname;
          thname << "Decay_" << ( *itr )->name();
          if ( ( *itr )->is_shadowrealm )
            threadhelp::start_thread( decay_thread_shadow, thname.str().c_str(), (void*)( *itr ) );
          else
            threadhelp::start_thread( decay_thread, thname.str().c_str(), (void*)( *itr ) );
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

#ifndef _WIN32
      //checkpoint( "start catch_signals thread" );
      //start_thread( catch_signals_thread, "CatchSignals" );
#endif
    }

    void check_incoming_data( void )
    {
      unsigned cli;
      SOCKET nfds = 0;
      FD_ZERO( &recv_fd );
      FD_ZERO( &err_fd );
      FD_ZERO( &send_fd );

      FD_SET( listen_socket, &recv_fd );
#ifndef _WIN32
      nfds = listen_socket + 1;
#endif

      for ( cli = 0; cli < clients.size(); cli++ )
      {
        Network::Client *client = clients[cli];

        FD_SET( client->csocket, &recv_fd );
        FD_SET( client->csocket, &err_fd );
        if ( client->have_queued_data() )
          FD_SET( client->csocket, &send_fd );

        if ( (SOCKET)( client->csocket + 1 ) > nfds )
          nfds = client->csocket + 1;
      }

      int res;
      do
      {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = config.select_timeout_usecs;
        res = select( static_cast<int>( nfds ), &recv_fd, &send_fd, &err_fd, &select_timeout );
      } while ( res < 0 && !Clib::exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );


      if ( res <= 0 )
        return;

      for ( cli = 0; cli < clients.size(); cli++ )
      {
        Network::Client *client = clients[cli];

        if ( !client->isReallyConnected() )
          continue;

        if ( FD_ISSET( client->csocket, &err_fd ) )
        {
          client->forceDisconnect();
        }

        if ( FD_ISSET( client->csocket, &recv_fd ) )
        {
          process_data( client );
        }

        if ( client->have_queued_data() && FD_ISSET( client->csocket, &send_fd ) )
        {
          client->send_queued_data();
        }
      }

      kill_disconnected_clients();

      if ( FD_ISSET( listen_socket, &recv_fd ) )
      {
        struct sockaddr client_addr; // inet_addr
        socklen_t addrlen = sizeof client_addr;
        SOCKET client_socket = accept( listen_socket, &client_addr, &addrlen );
        if ( client_socket == INVALID_SOCKET )
          return;

        Network::apply_socket_options( client_socket );
        if ( config.disable_nagle )
        {
          Network::disable_nagle( client_socket );
        }

        fmt::Writer tmp;
        tmp.Format( "Client connected from {}\n" ) << Network::AddressToString( &client_addr );
        INFO_PRINT << tmp.c_str();
        if ( config.loglevel >= 2 )
          POLLOG << tmp.c_str();

        Network::Client *client = new Network::Client( Network::uo_client_interface, config.client_encryption_version );
        client->csocket = client_socket;
        memcpy( &client->ipaddr, &client_addr, sizeof client->ipaddr );
        // Added null setting for pre-char selection checks using NULL validation
        client->acct = NULL;

        clients.push_back( client );
        INFO_PRINT << "Client connected (Total: " << clients.size() << ")\n";
      }
    }
#if REFPTR_DEBUG
    unsigned int ref_counted::_ctor_calls;
#endif

    void clear_script_storage()
    {
      scrstore.clear();
    }

    void display_unreaped_orphan_instances();

    void display_reftypes();

    void display_leftover_objects()
    {
      Bscript::display_executor_instances();
      display_unreaped_orphan_instances();
#if BOBJECTIMP_DEBUG
      Bscript::display_bobjectimp_instances( );
#endif
      display_reftypes();
      ofstream ofs;
      Clib::OFStreamWriter sw( &ofs );
      sw.init( "leftovers.txt" );
      objecthash.PrintContents( sw );
      fmt::Writer tmp;
      if ( uobject_count != 0 )
        tmp << "Remaining UObjects: " << uobject_count << "\n";
      if ( ucharacter_count != 0 )
        tmp << "Remaining Mobiles: " << ucharacter_count << "\n";
      if ( npc_count != 0 )
        tmp << "Remaining NPCs: " << npc_count << "\n";
      if ( uitem_count != 0 )
        tmp << "Remaining Items: " << uitem_count << "\n";
      if ( umulti_count != 0 )
        tmp << "Remaining Multis: " << umulti_count << "\n";
      if ( unreaped_orphans != 0 )
        tmp << "Unreaped orphans: " << unreaped_orphans << "\n";
      if ( uobj_count_echrref != 0 )
        tmp << "Remaining EChrRef objects: " << uobj_count_echrref << "\n";
      if ( Bscript::executor_count )
        tmp << "Remaining Executors: " << Bscript::executor_count << "\n";
      if ( Bscript::eobject_imp_count )
        tmp << "Remaining script objectimps: " << Bscript::eobject_imp_count << "\n";
      INFO_PRINT << tmp.c_str();
      if ( !existing_items.empty() )
      {
        for (const auto &item : existing_items)
        {
          INFO_PRINT << "Remaining item: " << item->serial << ": " << item->name() << "\n";
        }
      }
    }

    void run_start_scripts()
    {
      INFO_PRINT << "Running startup script.\n";
      run_script_to_completion( "start" );
      for ( const auto &pkg : Plib::packages )
      {
        string scriptname = pkg->dir() + "start.ecl";

        if ( Clib::FileExists( scriptname.c_str() ) )
        {
          ScriptDef script( "start", pkg, "" );
          Bscript::BObject obj( run_script_to_completion( script ) );
        }
      }
      INFO_PRINT << "Startup script complete.\n";
    }

#ifdef _WIN32
    typedef BOOL( WINAPI *DynHeapSetInformation ) (
      PVOID HeapHandle,
      HEAP_INFORMATION_CLASS HeapInformationClass,
      PVOID HeapInformation,
      SIZE_T HeapInformationLength
      );

    const char* Use_low_fragmentation_Heap()
    {
      if ( ssopt.use_win_lfh )
      {
        HINSTANCE hKernel32;

        hKernel32 = LoadLibrary( "Kernel32" );
        if ( hKernel32 != NULL )
        {
          DynHeapSetInformation ProcAdd;
          ProcAdd = (DynHeapSetInformation)GetProcAddress( hKernel32, "HeapSetInformation" );
          if ( ProcAdd != NULL )
          {
            ULONG HeapFragValue = 2;

            if ( (ProcAdd)( GetProcessHeap(), HeapCompatibilityInformation, &HeapFragValue, sizeof( HeapFragValue ) ) )
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
    void Check_libc_version( )
    {
      const char* libc_version = gnu_get_libc_version( );

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
        POLLOG_ERROR << "Error in analyzing libc version string [" << libc_version << "]. Please contact Core-Team.\n";

      if ( main_version * 100000000 + sub_version * 10000 + build >= 2 * 100000000 + 3 * 10000 + 2 )
        POLLOG_INFO << "Found libc " << libc_version << " - ok\n";
      else
        POLLOG_ERROR << "Found libc " << libc_version << " - Please update to 2.3.2 or above.\n";
    }
#endif


  } // namespace Core

  void polcleanup()
  {
    INFO_PRINT << "Initiating POL Cleanup....\n";

    for ( Core::Clients::iterator itr = Core::clients.begin(), end = Core::clients.end(); itr != end; ++itr )
    {
      Network::Client* sd_client = *itr;
      sd_client->forceDisconnect();
    }
    Core::kill_disconnected_clients();

    Core::deinit_ipc_vars();

    if ( Core::config.log_script_cycles )
      Core::log_all_script_cycle_counts( false );

    Core::checkpoint( "cleaning up vars" );
    Core::cleanup_vars();
    Core::checkpoint( "cleaning up scripts" );
    Core::cleanup_scripts();

    // scripts remove their listening points when they exit..
    // so there should be no listening points to clean up.
    Core::checkpoint( "cleaning listen points" );
    Core::clear_listen_points();

    Network::unload_aux_services(); // Nando - 2009-01-19

    Core::unload_other_objects();
    Items::unload_itemdesc_scripts();
    Core::unload_system_hooks();
    Core::UnloadAllConfigFiles();

    Plib::unload_packages(); // Nando - 2009-01-19
    Core::unload_npc_templates();  //quick and nasty fix until npcdesc usage is rewritten Turley 2012-08-27: moved before objecthash due to npc-method_script cleanup

    Bscript::UninitObject::ReleaseSharedInstance();
    Core::objecthash.Clear();
    Core::display_leftover_objects();

    Core::checkpoint( "unloading data" );
    Core::unload_data();

    Clib::MD5_Cleanup();

    Core::checkpoint( "misc cleanup" );

    Core::clear_script_storage();

#ifdef _WIN32
    closesocket( Core::listen_socket );
#else
    close( Core::listen_socket ); // shutdown( listen_socket, 2 ); ??
#endif
    Network::deinit_sockets_library();

    Core::checkpoint( "end of xmain2" );

#ifdef __linux__
    unlink( ( Core::config.pidfile_path + "pol.pid" ).c_str( ) );
#endif
  }

  int xmain_inner( int argc, char *argv[] )
  {
#ifdef _WIN32
    Clib::MiniDumper::Initialize();
    // Aug. 15, 2006 Austin
    // Added atexit() call to remove the tray icon.
    atexit( Core::ShutdownSystemTrayHandling );
#else
#ifdef __linux__
    ofstream polpid;

    polpid.open( ( Core::config.pidfile_path + "pol.pid" ).c_str( ), ios::out | ios::trunc );

    if ( polpid.is_open( ) )
      polpid << Clib::decint( getpid( ) );
    else
      INFO_PRINT << "Cannot create pid file in " << Core::config.pidfile_path << "\n";

    polpid.close( );
#endif
#endif

    int res;

    // for profiling:
    // chdir( "d:\\pol" );
    // PrintAllocationData(); 

    Clib::MakeDirectory( "log" );

    POLLOG_INFO << progverstr << " (" << polbuildtag << ")"
      << "\ncompiled on " << compiledate << " " << compiletime
      << "\nCopyright (C) 1993-2014 Eric N. Swanson"
      << "\n\n";

#ifndef NDEBUG
    POLLOG_INFO << "Sizes: \n"
      << "   UObject:    " << sizeof( Core::UObject ) << "\n"
      << "   Item:       " << sizeof( Items::Item ) << "\n"
      << "   UContainer: " << sizeof( Core::UContainer ) << "\n"
      << "   Character:  " << sizeof( Mobile::Character ) << "\n"
      << "   Client:     " << sizeof( Network::Client ) << "\n"
      << "   NPC:        " << sizeof( Core::NPC ) << "\n";

#ifdef __unix__
#ifdef PTHREAD_THREADS_MAX
    POLLOG_INFO << "   Max Threads: " << PTHREAD_THREADS_MAX << "\n";
#endif
#endif
    POLLOG_INFO << "\n";
#endif
#ifndef __clang__
    int max_threads = omp_get_max_threads();
    if ( max_threads > 1 )
    {
      max_threads /= 2;
      max_threads = std::max( 2, max_threads );
    }
    POLLOG_INFO << "Using " << max_threads << " out of " << omp_get_max_threads() << " worldsave threads\n";
#endif

    Core::checkpoint( "installing signal handlers" );
    Core::install_signal_handlers();

    Core::checkpoint( "starting POL clocks" );
    Core::start_pol_clocks();
    Core::pause_pol_clocks();

    POLLOG_INFO << "Reading Configuration.\n";

    Core::gflag_in_system_startup = true;

    Core::checkpoint( "reading pol.cfg" );
    Core::read_pol_config( true );

    Core::checkpoint( "reading config/bannedips.cfg" );
    Network::read_bannedips_config( true );

    Core::checkpoint( "reading servspecopt.cfg" );
    Core::read_servspecopt();

    Core::checkpoint( "reading extobj.cfg" );
    Core::read_extobj();

#ifdef _WIN32
    Core::checkpoint( Core::Use_low_fragmentation_Heap() );
#endif

#ifdef __linux__
    Core::checkpoint( "checking libc version" );
    Core::Check_libc_version( );
#endif

    Core::checkpoint( "init default itemdesc defaults" );
    Items::empty_itemdesc.doubleclick_range = Core::ssopt.default_doubleclick_range;
    Items::empty_itemdesc.decay_time = Core::ssopt.default_decay_time;

    Core::checkpoint( "loading POL map file" );
    if ( !Core::load_realms() )
    {
      POLLOG_ERROR << "Unable to load Realms. Please make sure your Realms have been generated by UOConvert and your RealmDataPath is set correctly in Pol.cfg.\n";
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
    Core::read_menus();

    Core::checkpoint( "loading intrinsic weapons" );
    Items::load_intrinsic_weapons();
    Core::checkpoint( "reading gameservers" );
    Core::read_gameservers();
    Core::checkpoint( "reading starting locations" );
    Core::read_starting_locations();

    if ( argc > 1 )
    {
      POLLOG_INFO << "Running POL test suite.\n";
      Core::run_pol_tests();
      Core::cancel_all_trades();
      Core::stop_gameclock();
      polcleanup();
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


    Items::allocate_intrinsic_weapon_serials();
    Core::gflag_in_system_startup = false;

    // PrintAllocationData();

    //onetime_create_stubdata();

    Core::checkpoint( "running start scripts" );
    Core::run_start_scripts();

    Core::checkpoint( "starting client listeners" );
    Core::start_uo_client_listeners();

    if ( Core::config.listen_port )
    {
      if ( Core::config.multithread )
      {
        // TODO: remove this warning after some releases...

        POLLOG_ERROR << "\n\n"
          << "+----------------------------------------------------------------------+\n"
          << "| Option ListenPort in pol.cfg is now only for non-multithreading      |\n"
          << "| systems. If you still haven't done it, please read the documentation |\n"
          << "| on how to create a uoclients.cfg.                                    |\n"
          << "+----------------------------------------------------------------------+\n"
          << "\n\n";

        throw runtime_error( "ListenPort is no longer used for multithreading programs (Multithread == 1)." );
      }
      Core::checkpoint( "opening listen socket" );
      Core::listen_socket = Network::open_listen_socket( Core::config.listen_port );
      if ( Core::listen_socket == INVALID_SOCKET )
      {
        POLLOG_ERROR << "Unable to listen on socket " << Core::config.listen_port << "\n";
        return 1;
      }
    }

    //	if( 1 )
    {
      POLLOG_INFO << "Initialization complete.  POL is active.  Ctrl-C to stop.\n\n";
    }
    //if( 1 )
    {
      DEINIT_STARTLOG();
    }
    POLLOG.Format( "{0:s} ({1:s}) compiled on {2:s} {3:s} running.\n" ) << progverstr << polbuildtag << compiledate << compiletime;
    //if( 1 )
    {
      POLLOG_INFO << "Game is active.\n";
    }
    Core::CoreSetSysTrayToolTip( "Running", Core::ToolTipPrioritySystem );

    //goto skip;

    Core::restart_pol_clocks();
    Core::polclock_checkin();

    // this is done right after reading globals from pol.txt:
    //checkpoint( "starting game clock" );
    //start_gameclock();

    Core::checkpoint( "starting periodic tasks" );
    Core::start_tasks();

    if ( Core::config.multithread )
    {
      Core::checkpoint( "starting threads" );
      Core::start_threads();
      Network::start_aux_services();

#ifdef _WIN32
      Core::console_thread();
      Core::checkpoint( "exit signal detected" );
      Core::CoreSetSysTrayToolTip( "Shutting down", Core::ToolTipPriorityShutdown );
#else
      // On Linux, signals are directed to a particular thread, if we use pthread_sigmask like we're supposed to.
      // therefore, we have to do this signal checking in this thread.
      threadhelp::start_thread( Core::console_thread, "Console" );

      Core::catch_signals_thread( );
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
    }
    else
    {
      Core::polclock_t sleeptime;
      bool activity;
      while ( !Clib::exit_signalled )
      {
        Core::last_checkpoint = "receiving TCP/IP data";
        Core::check_incoming_data();
        Core::last_checkpoint = "running scheduled tasks";
        Core::check_scheduled_tasks( &sleeptime, &activity );
        Core::last_checkpoint = "stepping scripts";
        Core::step_scripts( &sleeptime, &activity );
        Core::last_checkpoint = "performing decay";
        if ( Core::ssopt.decay_items )
          Core::decay_items();
        Core::last_checkpoint = "reaping objects";
        Core::objecthash.Reap();
        Core::last_checkpoint = "restarting clients";

        Core::restart_all_clients();

        ++Core::rotations;
      }
    }
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
        savetype = Core::config.assertion_shutdown_save_type;
      else
        savetype = Core::config.shutdown_save_type;

      // TODO: full save if incremental_saves_disabled ?
      // otherwise could have really, really bad timewarps
      Tools::Timer<> timer;
      if ( savetype == Core::SAVE_FULL )
        Core::write_data( dirty, clean, elapsed_ms );
      else
        Core::save_incremental( dirty, clean, elapsed_ms );
      Core::SaveContext::ready();
      POLLOG_INFO << "Data save completed in " << elapsed_ms << " ms. " << timer.ellapsed() << " total.\n";
    }
    else
    {
      if ( Clib::passert_shutdown_due_to_assertion && Clib::passert_nosave )
        POLLOG_INFO << "Not writing data due to assertion failure.\n";
      else if ( Core::config.inhibit_saves )
        POLLOG_INFO << "Not writing data due to pol.cfg InhibitSaves=1 setting.\n";
    }
    polcleanup();
    return 0;
  }

  int xmain_outer( int argc, char *argv[] )
  {
    try
    {
      return xmain_inner( argc, argv );
    }
    catch ( std::exception& )
    {
      if ( Core::last_checkpoint != NULL )
      {
        POLLOG_INFO << "Server Shutdown: " << Core::last_checkpoint << "\n";
        //pol_sleep_ms( 10000 );
      }
      polcleanup();
      Core::objecthash.Clear();

      throw;
    }
  }

#ifndef _WIN32
  int xmain( int argc, char *argv[] )
  {
    strcpy( progverstr, polverstr );
    strcpy( buildtagstr, polbuildtag );
    progver = polver;

    return xmain_outer( argc, argv );
  }
#endif
}

