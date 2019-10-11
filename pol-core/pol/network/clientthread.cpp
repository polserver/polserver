#include "clientthread.h"

#include <errno.h>
#include <exception>
#include <stddef.h>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/esignal.h"
#include "../../clib/fdump.h"
#include "../../clib/logfacility.h"
#include "../../clib/network/singlepoller.h"
#include "../../clib/passert.h"
#include "../../clib/spinlock.h"
#include "../../clib/stlutil.h"
#include "../../plib/systemstate.h"
#include "../accounts/account.h"
#include "../clib/network/sockets.h"
#include "../core.h"
#include "../crypt/cryptbase.h"
#include "../mobile/charactr.h"
#include "../polcfg.h"
#include "../polclock.h"
#include "../polsem.h"
#include "../schedule.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "cgdata.h"  // This might not be needed if the client has a clear_gd() method
#include "client.h"
#include "msgfiltr.h"  // Client could also have a method client->is_msg_allowed(), for example. Then this is not needed here.
#include "msghandl.h"
#include "packethelper.h"
#include "packets.h"
#include "pktboth.h"
#include "pktbothid.h"
#include "pktdef.h"
#include "pktinid.h"
#include <format/format.h>

#define CLIENT_CHECKPOINT( x ) client->checkpoint = x

#ifdef _MSC_VER
#pragma warning( disable : 4127 )  // conditional expression is constant, because of FD_SET
#endif

namespace Pol
{
namespace Core
{
// This function below is defined (for now) in pol.cpp. That's ugly.
void call_chr_scripts( Mobile::Character* chr, const std::string& root_script_ecl,
                       const std::string& pkg_script_ecl, bool offline = false );


void report_weird_packet( Network::Client* client, const std::string& why );  // Defined below

void set_polling_timeouts( Clib::SinglePoller& poller, bool single_threaded_login )
{
  if ( !single_threaded_login )
  {
    poller.set_timeout( 2, 0 );
  }
  else
  {
    poller.set_timeout( 0, Plib::systemstate.config.select_timeout_usecs );
  }
}

bool client_io_thread( Network::Client* client, bool login )
{
  int checkpoint = 0;
  int nidle = 0;
  client->last_packet_at = polclock();
  client->last_activity_at = polclock();

  if ( !login && Plib::systemstate.config.loglevel >= 11 )
  {
    POLLOG.Format( "Network::Client#{} i/o thread starting\n" ) << client->instance_;
  }

  CLIENT_CHECKPOINT( 0 );
  try
  {
    Clib::SinglePoller clientpoller( client->csocket );
    set_polling_timeouts( clientpoller, login );

    while ( !Clib::exit_signalled && client->isReallyConnected() )
    {
      CLIENT_CHECKPOINT( 1 );
      checkpoint = 1;
      if ( !clientpoller.prepare( client->have_queued_data() ) )
      {
        POLLOG_INFO.Format( "Client#{}: ERROR - couldn't poll socket={}\n" )
            << client->instance_ << client->csocket;

        if ( client->csocket != INVALID_SOCKET )
          client->forceDisconnect();

        break;
      }
      checkpoint = 2;

      int res = 0;
      do
      {
        CLIENT_CHECKPOINT( 2 );
        res = clientpoller.wait_for_events();
        CLIENT_CHECKPOINT( 3 );
      } while ( res < 0 && !Clib::exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );
      checkpoint = 3;

      if ( res < 0 )
      {
        int sckerr = socket_errno;
        POLLOG.Format( "Client#{}: select res={}, sckerr={}\n" )
            << client->instance_ << res << sckerr;
        break;
      }
      else if ( res == 0 )
      {
        if ( ( !client->chr ||
               client->chr->cmdlevel() < Plib::systemstate.config.min_cmdlvl_ignore_inactivity ) &&
             Plib::systemstate.config.inactivity_warning_timeout &&
             Plib::systemstate.config.inactivity_disconnect_timeout )
        {
          ++nidle;
          if ( nidle == 30 * Plib::systemstate.config.inactivity_warning_timeout )
          {
            CLIENT_CHECKPOINT( 4 );
            PolLock lck;  // multithread
            Network::PktHelper::PacketOut<Network::PktOut_53> msg;
            msg->Write<u8>( PKTOUT_53_WARN_CHARACTER_IDLE );
            CLIENT_CHECKPOINT( 5 );
            msg.Send( client );
            CLIENT_CHECKPOINT( 18 );
            if ( client->pause_count )
              client->restart();
          }
          else if ( nidle == 30 * Plib::systemstate.config.inactivity_disconnect_timeout )
          {
            client->forceDisconnect();
          }
        }
      }

      CLIENT_CHECKPOINT( 19 );
      if ( !client->isReallyConnected() )
        break;

      if ( clientpoller.error() )
      {
        client->forceDisconnect();
        break;
      }

      // region Speedhack
      if ( !client->movementqueue.empty() )  // not empty then process the first packet
      {
        PolLock lck;  // multithread
        Network::PacketThrottler pkt = client->movementqueue.front();
        if ( client->SpeedHackPrevention( false ) )
        {
          if ( client->isReallyConnected() )
          {
            unsigned char msgtype = pkt.pktbuffer[0];
            Network::MSG_HANDLER packetHandler =
                Network::PacketRegistry::find_handler( msgtype, client );
            try
            {
              INFO_PRINT_TRACE( 10 ) << "Client#" << client->instance_ << ": message 0x"
                                     << fmt::hexu( msgtype ) << "\n";
              CLIENT_CHECKPOINT( 26 );
              packetHandler.func( client, pkt.pktbuffer );
              CLIENT_CHECKPOINT( 27 );
              restart_all_clients();
            }
            catch ( std::exception& ex )
            {
              POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                  << client->instance_ << (int)msgtype << ex.what();
              fmt::Writer tmp;
              Clib::fdump( tmp, pkt.pktbuffer, 7 );
              POLLOG << tmp.str() << "\n";
              restart_all_clients();
              throw;
            }
          }
          client->movementqueue.pop();
        }
      }
      // endregion Speedhack

      if ( clientpoller.incoming() )
      {
        checkpoint = 4;
        CLIENT_CHECKPOINT( 6 );
        if ( process_data( client ) )
        {
          CLIENT_CHECKPOINT( 17 );
          PolLock lck;

          // reset packet timer
          client->last_packet_at = polclock();
          if ( !check_inactivity( client ) )
          {
            nidle = 0;
            client->last_activity_at = polclock();
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
      if ( ( ( polclock_now - client->last_packet_at ) / POLCLOCKS_PER_SEC ) >= 120 )  // 2 mins
      {
        client->forceDisconnect();
        break;
      }

      if ( client->have_queued_data() && clientpoller.writable() )
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
  catch ( std::string& str )
  {
    POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
        << client->instance_ << str << checkpoint;
  }
  catch ( const char* msg )
  {
    POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
        << client->instance_ << msg << checkpoint;
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERROR.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={})\n" )
        << client->instance_ << ex.what() << checkpoint;
  }
  CLIENT_CHECKPOINT( 20 );

  if ( login && client->isConnected() )
    return true;
  POLLOG.Format( "Client#{} ({}): disconnected (account {})\n" )
      << client->instance_ << client->ipaddrAsString()
      << ( ( client->acct != nullptr ) ? client->acct->name() : "unknown" );


  try
  {
    //    if (1)
    {
      CLIENT_CHECKPOINT( 9 );
      PolLock lck;
      client->unregister();
      INFO_PRINT << "Client disconnected from " << client->ipaddrAsString() << " ("
                 << networkManager.clients.size() << "/" << networkManager.getNumberOfLoginClients()
                 << " connections)\n";

      CoreSetSysTrayToolTip( Clib::tostring( networkManager.clients.size() ) + " clients connected",
                             ToolTipPrioritySystem );
    }

    checkpoint = 8;
    CLIENT_CHECKPOINT( 10 );
    if ( client->chr )
    {
      //      if (1)
      int seconds_wait = 0;
      {
        CLIENT_CHECKPOINT( 11 );
        PolLock lck;

        if ( client->chr )
        {
          client->chr->disconnect_cleanup();
          client->gd->clear();
          client->chr->connected( false );
          ScriptDef sd;
          sd.quickconfig( "scripts/misc/logofftest.ecl" );
          if ( sd.exists() )
          {
            CLIENT_CHECKPOINT( 12 );
            Bscript::BObject bobj(
                run_script_to_completion( sd, new Module::ECharacterRefObjImp( client->chr ) ) );
            if ( bobj.isa( Bscript::BObjectImp::OTLong ) )
            {
              const Bscript::BLong* blong = static_cast<const Bscript::BLong*>( bobj.impptr() );
              seconds_wait = blong->value();
            }
          }
        }
      }

      polclock_t when_logoff = client->last_activity_at + seconds_wait * POLCLOCKS_PER_SEC;

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
      //      if (1)
      {
        PolLock lck;
        if ( client->chr )
        {
          Mobile::Character* chr = client->chr;
          CLIENT_CHECKPOINT( 16 );
          call_chr_scripts( chr, "scripts/misc/logoff.ecl", "logoff.ecl" );
          if ( chr->realm )
          {
            chr->realm->notify_left( *chr );
          }
        }
      }
    }
  }
  catch ( std::exception& ex )
  {
    POLLOG.Format( "Client#{}: Exception in i/o thread: {}! (checkpoint={}, what={})\n" )
        << client->instance_ << checkpoint << ex.what();
  }

  // queue delete of client ptr see method doc for reason
  Core::networkManager.clientTransmit->QueueDelete( client );
  return false;
}

bool valid_message_length( Network::Client* client, unsigned int length )
{
  if ( length > sizeof client->buffer )
  {
    handle_humongous_packet( client, client->message_length );
    return false;
  }
  if ( length < 3 )
  {
    report_weird_packet( client, "Too-short message" );
    return false;
  }
  return true;
}

// bool - return true when a message was processed.
bool process_data( Network::Client* client )
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
    if ( client->bytes_received < 1 )  // this really should never happen.
    {
      client->forceDisconnect();
      return false;
    }

    unsigned char msgtype = client->buffer[0];
    client->last_msgtype = msgtype;  // CNXBUG
    if ( Plib::systemstate.config.verbose )
      INFO_PRINT.Format( "Incoming msg type: 0x{:X}\n" ) << (int)msgtype;

    if ( !Network::PacketRegistry::is_defined( msgtype ) )
    {
      handle_undefined_packet( client );
      return false;  // remain in RECV_STATE_MSGTYPE_WAIT
    }

    Network::MSG_HANDLER packetHandler = Network::PacketRegistry::find_handler( msgtype, client );
    if ( packetHandler.msglen == MSGLEN_2BYTELEN_DATA )
    {
      client->recv_state = Network::Client::RECV_STATE_MSGLEN_WAIT;
    }
    else
    {
      passert( packetHandler.msglen > 0 );

      client->recv_state = Network::Client::RECV_STATE_MSGDATA_WAIT;
      client->message_length = packetHandler.msglen;
    }

  } /* endif of RECV_STATE_MSGTYPE_WAIT */

  if ( client->recv_state == Network::Client::RECV_STATE_MSGLEN_WAIT )
  {
    client->recv_remaining( 3 );
    CLIENT_CHECKPOINT( 23 );
    if ( client->bytes_received == 3 )  // the length bytes were available.
    {
      // MSG is [MSGTYPE] [LENHI] [LENLO] [DATA ... ]
      client->message_length = ( client->buffer[1] << 8 ) + client->buffer[2];

      if ( !valid_message_length( client, client->message_length ) )
      {
        // If the reported length is too short (less than 3 bytes) or
        // too big (larger than the client buffer), something very odd
        // happened.
        client->forceDisconnect();
        return false;
      }
      client->recv_state = Network::Client::RECV_STATE_MSGDATA_WAIT;
    }
    // else keep waiting.
  } /* endif of RECV_STATE_MSGLEN_WAIT */

  if ( client->recv_state == Network::Client::RECV_STATE_MSGDATA_WAIT )
  {
    CLIENT_CHECKPOINT( 24 );
    client->recv_remaining( client->message_length );
    CLIENT_CHECKPOINT( 25 );
    if ( client->bytes_received == client->message_length )  // we have the whole message
    {
      unsigned char msgtype = client->buffer[0];
      networkManager.iostats.received[msgtype].count++;
      networkManager.iostats.received[msgtype].bytes += client->message_length;
      {
        Clib::SpinLockGuard guard( client->_fpLog_lock );
        if ( !client->fpLog.empty() )
        {
          fmt::Writer tmp;
          tmp << "Client -> Server: 0x" << fmt::hexu( msgtype ) << ", " << client->message_length
              << " bytes\n";
          Clib::fdump( tmp, &client->buffer, client->message_length );
          FLEXLOG( client->fpLog ) << tmp.str() << "\n";
        }
      }

      if ( Plib::systemstate.config.verbose )
        INFO_PRINT.Format( "Message Received: Type 0x{:X}, Length {} bytes\n" )
            << (int)msgtype << client->message_length;

      PolLock lck;  // multithread
      // it can happen that a client gets disconnected while waiting for the lock.
      if ( client->isConnected() )
      {
        if ( client->msgtype_filter->msgtype_allowed[msgtype] )
        {
          // region Speedhack
          if ( ( settingsManager.ssopt.speedhack_prevention ) && ( msgtype == PKTIN_02_ID ) )
          {
            if ( !client->SpeedHackPrevention() )
            {
              // client->SpeedHackPrevention() added packet to queue
              client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
              CLIENT_CHECKPOINT( 28 );
              return true;
            }
          }
          // endregion Speedhack


          Network::MSG_HANDLER packetHandler =
              Network::PacketRegistry::find_handler( msgtype, client );
          passert( packetHandler.msglen != 0 );

          try
          {
            INFO_PRINT_TRACE( 10 )
                << "Client#" << client->instance_ << ": message 0x" << fmt::hexu( msgtype ) << "\n";
            CLIENT_CHECKPOINT( 26 );
            packetHandler.func( client, client->buffer );
            CLIENT_CHECKPOINT( 27 );
            restart_all_clients();
          }
          catch ( std::exception& ex )
          {
            POLLOG_ERROR.Format( "Client#{}: Exception in message handler 0x{:X}: {}\n" )
                << client->instance_ << (int)msgtype << ex.what();
            fmt::Writer tmp;
            Clib::fdump( tmp, client->buffer, client->bytes_received );
            POLLOG << tmp.str() << "\n";
            restart_all_clients();
            throw;
          }
        }
        else
        {
          POLLOG_ERROR.Format( "Client#{} ({}, Acct {}) sent non-allowed message type 0x{:X}.\n" )
              << client->instance_ << client->ipaddrAsString()
              << ( client->acct ? client->acct->name() : "unknown" ) << (int)msgtype;
        }
      }
      client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
      CLIENT_CHECKPOINT( 28 );
      return true;
    }
    // else keep waiting
  } /* endif RECV_STATE_MSGDATA_WAIT */
  else if ( client->recv_state == Network::Client::RECV_STATE_CRYPTSEED_WAIT )
  {  // The abnormal case.
    // The first four bytes after connection are the
    // crypto seed
    client->recv_remaining_nocrypt( 4 );

    if ( client->bytes_received == 4 )
    {
      /* The first four bytes transmitted are the encryption seed */
      unsigned char cstype = client->buffer[0];

      if ( ( client->buffer[0] == 0xff ) && ( client->buffer[1] == 0xff ) &&
           ( client->buffer[2] == 0xff ) && ( client->buffer[3] == 0xff ) )
      {
        if ( Plib::systemstate.config.verbose )
        {
          INFO_PRINT.Format( "UOKR Seed Message Received: Type 0x{:X}\n" ) << (int)cstype;
        }
        Network::PktHelper::PacketOut<Network::PktOut_E3> msg;
        msg->WriteFlipped<u16>( 77u );
        msg->WriteFlipped<u32>( 0x03u );
        msg->Write<u8>( 0x02u );
        msg->Write<u8>( 0x01u );
        msg->Write<u8>( 0x03u );
        msg->WriteFlipped<u32>( 0x13u );
        msg->Write<u8>( 0x02u );
        msg->Write<u8>( 0x11u );
        msg->Write<u8>( 0x00u );
        msg->Write<u8>( 0xfcu );
        msg->Write<u8>( 0x2fu );
        msg->Write<u8>( 0xe3u );
        msg->Write<u8>( 0x81u );
        msg->Write<u8>( 0x93u );
        msg->Write<u8>( 0xcbu );
        msg->Write<u8>( 0xafu );
        msg->Write<u8>( 0x98u );
        msg->Write<u8>( 0xddu );
        msg->Write<u8>( 0x83u );
        msg->Write<u8>( 0x13u );
        msg->Write<u8>( 0xd2u );
        msg->Write<u8>( 0x9eu );
        msg->Write<u8>( 0xeau );
        msg->Write<u8>( 0xe4u );
        msg->Write<u8>( 0x13u );
        msg->WriteFlipped<u32>( 0x10u );
        msg->Write<u8>( 0x78u );
        msg->Write<u8>( 0x13u );
        msg->Write<u8>( 0xb7u );
        msg->Write<u8>( 0x7bu );
        msg->Write<u8>( 0xceu );
        msg->Write<u8>( 0xa8u );
        msg->Write<u8>( 0xd7u );
        msg->Write<u8>( 0xbcu );
        msg->Write<u8>( 0x52u );
        msg->Write<u8>( 0xdeu );
        msg->Write<u8>( 0x38u );
        msg->Write<u8>( 0x30u );
        msg->Write<u8>( 0xeau );
        msg->Write<u8>( 0xe9u );
        msg->Write<u8>( 0x1eu );
        msg->Write<u8>( 0xa3u );
        msg->WriteFlipped<u32>( 0x20u );
        msg->WriteFlipped<u32>( 0x10u );
        msg->Write<u8>( 0x5au );
        msg->Write<u8>( 0xceu );
        msg->Write<u8>( 0x3eu );
        msg->Write<u8>( 0xe3u );
        msg->Write<u8>( 0x97u );
        msg->Write<u8>( 0x92u );
        msg->Write<u8>( 0xe4u );
        msg->Write<u8>( 0x8au );
        msg->Write<u8>( 0xf1u );
        msg->Write<u8>( 0x9au );
        msg->Write<u8>( 0xd3u );
        msg->Write<u8>( 0x04u );
        msg->Write<u8>( 0x41u );
        msg->Write<u8>( 0x03u );
        msg->Write<u8>( 0xcbu );
        msg->Write<u8>( 0x53u );
        client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
        client->setClientType( Network::CLIENTTYPE_UOKR );  // UO:KR logging in
        msg.Send( client );
      }
      else if ( client->buffer[0] ==
                PKTIN_EF_ID )  // new seed since 6.0.5.0 (0xef should never appear in normal ipseed)
      {
        if ( Plib::systemstate.config.verbose )
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
    client->recv_remaining_nocrypt(
        21 );  // receive and send to handler to get directly the version
    if ( client->bytes_received == 21 )
    {
      client->recv_state = Network::Client::RECV_STATE_MSGTYPE_WAIT;
      unsigned char tempseed[4];
      tempseed[0] = client->buffer[1];
      tempseed[1] = client->buffer[2];
      tempseed[2] = client->buffer[3];
      tempseed[3] = client->buffer[4];
      client->cryptengine->Init( tempseed, Crypt::CCryptBase::typeLogin );
      Network::PacketRegistry::handle_msg( PKTIN_EF_ID, client, client->buffer );
    }
  }

  return false;
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

void report_weird_packet( Network::Client* client, const std::string& why )
{
  fmt::Writer tmp;
  tmp.Format( "Client#{}: {} type 0x{:X}, {} bytes (IP: {}, Account: {})\n" )
      << client->instance_ << why << (int)client->buffer[0] << client->bytes_received
      << client->ipaddrAsString()
      << ( ( client->acct != nullptr ) ? client->acct->name() : "None" );

  if ( client->bytes_received <= 64 )
  {
    Clib::fdump( tmp, client->buffer, client->bytes_received );
    POLLOG_INFO << tmp.str() << "\n";
  }
  else
  {
    INFO_PRINT << tmp.str();
    Clib::fdump( tmp, client->buffer, client->bytes_received );
    POLLOG << tmp.str() << "\n";
  }
}

// Called when a packet size is registered but the
// packet has no handler in the core
void handle_unknown_packet( Network::Client* client )
{
  if ( Plib::systemstate.config.display_unknown_packets )
    report_weird_packet( client, "Unknown packet" );
}

// Called when POL receives an undefined packet.
// Those have no registered size, so we must guess.
void handle_undefined_packet( Network::Client* client )
{
  int msgtype = (int)client->buffer[0];
  INFO_PRINT.Format( "Undefined message type 0x{:X}\n" ) << msgtype;

  // Tries to read as much of it out as possible
  client->recv_remaining( sizeof client->buffer / 2 );

  report_weird_packet( client, "Unexpected message" );
}

// Handles variable-sized packets whose declared size is much larger than
// the receive buffer. This packet is most likely a client error, because
// the buffer should be big enough to handle anything sent by the known
// clients.
void handle_humongous_packet( Network::Client* client, unsigned int reported_size )
{
  // Tries to read as much of it out as possible
  // (the client will be disconnected, but this may
  // be useful for debugging)
  client->recv_remaining( sizeof client->buffer / 2 );

  fmt::Writer tmp;
  tmp.Format( "Humongous packet (length {})", reported_size );
  report_weird_packet( client, tmp.str() );
}
}  // namespace Core
}  // namespace Pol
