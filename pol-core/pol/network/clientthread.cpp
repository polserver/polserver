#include "clientthread.h"

#include <errno.h>
#include <exception>
#include <iterator>
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


#define CLIENT_CHECKPOINT( x ) client->session()->checkpoint = x
#define SESSION_CHECKPOINT( x ) session->checkpoint = x

namespace Pol::Core
{
// This function below is defined (for now) in pol.cpp. That's ugly.
void call_chr_scripts( Mobile::Character* chr, const std::string& root_script_ecl,
                       const std::string& pkg_script_ecl, bool offline = false );


void report_weird_packet( Network::ThreadedClient* session,
                          const std::string& why );  // Defined below

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

// Taking a reference to SinglePoller is ugly here. But io_step, io_loop and clientpoller will
// eventually move into the same class.
bool threadedclient_io_step( Network::ThreadedClient* session, Clib::SinglePoller& clientpoller,
                             int& nidle )
{
  SESSION_CHECKPOINT( 1 );
  if ( !clientpoller.prepare( session->have_queued_data() ) )
  {
    POLLOG_INFO( "Client#{}: ERROR - couldn't poll socket={}\n", session->myClient.instance_,
                 session->csocket );

    if ( session->csocket != INVALID_SOCKET )
      session->forceDisconnect();

    return false;
  }

  int res = 0;
  do
  {
    SESSION_CHECKPOINT( 2 );
    res = clientpoller.wait_for_events();
    SESSION_CHECKPOINT( 3 );
  } while ( res < 0 && !Clib::exit_signalled && socket_errno == SOCKET_ERRNO( EINTR ) );

  if ( res < 0 )
  {
    int sckerr = socket_errno;
    POLLOGLN( "Client#{}: select res={}, sckerr={}", session->myClient.instance_, res, sckerr );
    return false;
  }
  else if ( res == 0 )
  {
    if ( session->myClient.should_check_idle() )
    {
      ++nidle;
      if ( nidle == 30 * Plib::systemstate.config.inactivity_warning_timeout )
      {
        SESSION_CHECKPOINT( 4 );
        PolLock lck;  // multithread
        session->myClient.warn_idle();
      }
      else if ( nidle == 30 * Plib::systemstate.config.inactivity_disconnect_timeout )
      {
        session->forceDisconnect();
      }
    }
  }

  SESSION_CHECKPOINT( 19 );
  if ( !session->isReallyConnected() )
    return false;

  if ( clientpoller.error() )
  {
    session->forceDisconnect();
    return false;
  }

  // region Speedhack
  if ( session->has_delayed_packets() )  // not empty then process the first packet
  {
    PolLock lck;  // multithread
    session->process_delayed_packets();
  }
  // endregion Speedhack

  if ( clientpoller.incoming() )
  {
    SESSION_CHECKPOINT( 6 );
    if ( process_data( session ) )
    {
      SESSION_CHECKPOINT( 17 );
      PolLock lck;

      // reset packet timer
      session->last_packet_at = polclock();
      if ( !check_inactivity( session ) )
      {
        nidle = 0;
        session->last_activity_at = polclock();
      }

      SESSION_CHECKPOINT( 7 );
      send_pulse();
      if ( TaskScheduler::is_dirty() )
        wake_tasks_thread();
    }
  }

  polclock_t polclock_now = polclock();
  if ( ( ( polclock_now - session->last_packet_at ) / POLCLOCKS_PER_SEC ) >= 120 )  // 2 mins
  {
    session->forceDisconnect();
    return false;
  }

  if ( session->have_queued_data() && clientpoller.writable() )
  {
    PolLock lck;
    SESSION_CHECKPOINT( 8 );
    session->send_queued_data();
  }
  SESSION_CHECKPOINT( 21 );

  return true;
}  // namespace Pol::Core

void threadedclient_io_loop( Network::ThreadedClient* session, bool login )
{
  int nidle = 0;
  session->last_packet_at = polclock();
  session->last_activity_at = polclock();

  Clib::SinglePoller clientpoller( session->csocket );
  set_polling_timeouts( clientpoller, login );

  while ( !Clib::exit_signalled && session->isReallyConnected() )
  {
    if ( !threadedclient_io_step( session, clientpoller, nidle ) || login )
      break;
  }
}

// Sleeps taking exit_signalled into account
void threadedclient_sleep_until( polclock_t when_logoff )
{
  while ( !Clib::exit_signalled )
  {
    if ( polclock() >= when_logoff )
      break;
    pol_sleep_ms( 2000 );  // min(2000, when_logoff - polclock()) ?
  }
}

void threadedclient_io_finalize( Network::ThreadedClient* session )
{
  int seconds_wait = 0;
  {
    SESSION_CHECKPOINT( 9 );
    PolLock lck;
    seconds_wait = session->myClient.on_close();
  }

  SESSION_CHECKPOINT( 10 );
  if ( seconds_wait > 0 )
  {
    polclock_t when_logoff = session->last_activity_at + seconds_wait * POLCLOCKS_PER_SEC;
    threadedclient_sleep_until( when_logoff );
  }

  SESSION_CHECKPOINT( 15 );
  if ( session->myClient.chr )
  {
    PolLock lck;
    session->myClient.on_logoff();
  }
}

bool client_io_thread( Network::Client* client, bool login )
{
  if ( !login && Plib::systemstate.config.loglevel >= 11 )
  {
    POLLOGLN( "Network::Client#{} i/o thread starting", client->instance_ );
  }

  CLIENT_CHECKPOINT( 0 );
  try
  {
    threadedclient_io_loop( client->session(), login );
  }
  catch ( std::string& str )
  {
    POLLOG_ERRORLN( "Client#{}: Exception in i/o thread: {}! (checkpoint={})", client->instance_,
                    str, client->session()->checkpoint );
  }
  catch ( const char* msg )
  {
    POLLOG_ERRORLN( "Client#{}: Exception in i/o thread: {}! (checkpoint={})", client->instance_,
                    msg, client->session()->checkpoint );
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERRORLN( "Client#{}: Exception in i/o thread: {}! (checkpoint={})", client->instance_,
                    ex.what(), client->session()->checkpoint );
  }
  CLIENT_CHECKPOINT( 20 );

  if ( login && client->isConnected() )
    return true;

  POLLOGLN( "Client#{} ({}): disconnected (account {})", client->instance_,
            client->ipaddrAsString(),
            ( ( client->acct != nullptr ) ? client->acct->name() : "unknown" ) );

  try
  {
    threadedclient_io_finalize( client->session() );
  }
  catch ( std::exception& ex )
  {
    POLLOGLN( "Client#{}: Exception in i/o thread: {}! (checkpoint={}, what={})", client->instance_,
              client->session()->checkpoint, ex.what() );
  }

  // queue delete of client ptr see method doc for reason
  Core::networkManager.clientTransmit->QueueDelete( client );
  return false;
}

bool valid_message_length( Network::ThreadedClient* session, unsigned int length )
{
  if ( length > sizeof session->buffer )
  {
    handle_humongous_packet( session, session->message_length );
    return false;
  }
  if ( length < 3 )
  {
    report_weird_packet( session, "Too-short message" );
    return false;
  }
  return true;
}

// bool - return true when a message was processed.
bool process_data( Network::ThreadedClient* session )
{
  // NOTE: This is coded such that for normal messages, which are completely available,
  // this function will get the type, then the length, then the data, without having
  // to wait for a second or third call.
  // Also, the abnormal state, RECV_STATE_CRYPTSEED_WAIT, is handled at the end, so in
  // normal processing its code doesn't see the code path.
  passert( session->bufcheck1_AA == 0xAA );
  passert( session->bufcheck2_55 == 0x55 );
  if ( session->recv_state == Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT )
  {
    session->bytes_received = 0;
    session->recv_remaining( 1 );
    SESSION_CHECKPOINT( 22 );
    if ( session->bytes_received < 1 )  // this really should never happen.
    {
      session->forceDisconnect();
      return false;
    }

    unsigned char msgtype = session->buffer[0];
    session->last_msgtype = msgtype;  // CNXBUG
    if ( Plib::systemstate.config.verbose )
      INFO_PRINTLN( "Incoming msg type: {:#X}", (int)msgtype );

    if ( !Network::PacketRegistry::is_defined( msgtype ) )
    {
      handle_undefined_packet( session );
      return false;  // remain in RECV_STATE_MSGTYPE_WAIT
    }

    Network::MSG_HANDLER packetHandler =
        Network::PacketRegistry::find_handler( msgtype, &session->myClient );
    if ( packetHandler.msglen == MSGLEN_2BYTELEN_DATA )
    {
      session->recv_state = Network::ThreadedClient::RECV_STATE_MSGLEN_WAIT;
    }
    else
    {
      passert( packetHandler.msglen > 0 );

      session->recv_state = Network::ThreadedClient::RECV_STATE_MSGDATA_WAIT;
      session->message_length = packetHandler.msglen;
    }

  } /* endif of RECV_STATE_MSGTYPE_WAIT */

  if ( session->recv_state == Network::ThreadedClient::RECV_STATE_MSGLEN_WAIT )
  {
    session->recv_remaining( 3 );
    SESSION_CHECKPOINT( 23 );
    if ( session->bytes_received == 3 )  // the length bytes were available.
    {
      // MSG is [MSGTYPE] [LENHI] [LENLO] [DATA ... ]
      session->message_length = ( session->buffer[1] << 8 ) + session->buffer[2];

      if ( !valid_message_length( session, session->message_length ) )
      {
        // If the reported length is too short (less than 3 bytes) or
        // too big (larger than the client buffer), something very odd
        // happened.
        session->forceDisconnect();
        return false;
      }
      session->recv_state = Network::ThreadedClient::RECV_STATE_MSGDATA_WAIT;
    }
    // else keep waiting.
  } /* endif of RECV_STATE_MSGLEN_WAIT */

  if ( session->recv_state == Network::ThreadedClient::RECV_STATE_MSGDATA_WAIT )
  {
    SESSION_CHECKPOINT( 24 );
    session->recv_remaining( session->message_length );
    SESSION_CHECKPOINT( 25 );
    if ( session->bytes_received == session->message_length )  // we have the whole message
    {
      unsigned char msgtype = session->buffer[0];
      networkManager.iostats.received[msgtype].count++;
      networkManager.iostats.received[msgtype].bytes += session->message_length;
      {
        // Consider if the client should do the logging via client->log_incoming(...) instead. Also,
        // can we avoid the SpinLock here?
        Clib::SpinLockGuard guard( session->_fpLog_lock );
        if ( !session->fpLog.empty() )
        {
          std::string tmp = fmt::format( "Client -> Server: {:#X}, {} bytes\n", msgtype,
                                         session->message_length );
          Clib::fdump( std::back_inserter( tmp ), &session->buffer, session->message_length );
          FLEXLOGLN( session->fpLog, tmp );
        }
      }

      if ( Plib::systemstate.config.verbose )
        INFO_PRINTLN( "Message Received: Type {:#X}, Length {} bytes", (int)msgtype,
                      session->message_length );

      PolLock lck;  // multithread
      // it can happen that a client gets disconnected while waiting for the lock.
      if ( session->isConnected() )
      {
        if ( session->msgtype_filter->msgtype_allowed[msgtype] )
        {
          // region Speedhack
          if ( ( settingsManager.ssopt.speedhack_prevention ) && ( msgtype == PKTIN_02_ID ) )
          {
            if ( !session->myClient.SpeedHackPrevention() )
            {
              // client->SpeedHackPrevention() added packet to queue
              session->recv_state = Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT;
              SESSION_CHECKPOINT( 28 );
              return true;
            }
          }
          // endregion Speedhack

          session->myClient.handle_msg( session->buffer, session->bytes_received );
        }
        else
        {
          // Such combinations of instance and acct happen quite often. Maybe this should become
          // Client->full_id() or something.
          POLLOG_ERRORLN( "Client#{} ({}, Acct {}) sent non-allowed message type {:#X}.",
                          session->myClient.instance_, session->ipaddrAsString(),
                          ( session->myClient.acct ? session->myClient.acct->name() : "unknown" ),
                          (int)msgtype );
        }
      }
      session->recv_state = Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT;
      SESSION_CHECKPOINT( 28 );
      return true;
    }
    // else keep waiting
  } /* endif RECV_STATE_MSGDATA_WAIT */
  else if ( session->recv_state == Network::ThreadedClient::RECV_STATE_CRYPTSEED_WAIT )
  {  // The abnormal case.
    // The first four bytes after connection are the
    // crypto seed
    session->recv_remaining_nocrypt( 4 );

    if ( session->bytes_received == 4 )
    {
      /* The first four bytes transmitted are the encryption seed */
      unsigned char cstype = session->buffer[0];

      if ( ( session->buffer[0] == 0xff ) && ( session->buffer[1] == 0xff ) &&
           ( session->buffer[2] == 0xff ) && ( session->buffer[3] == 0xff ) )
      {
        if ( Plib::systemstate.config.verbose )
        {
          INFO_PRINTLN( "UOKR Seed Message Received: Type {:#X}", (int)cstype );
        }
        session->myClient.send_KR_encryption_response();
        session->myClient.setClientType( Network::CLIENTTYPE_UOKR );  // UO:KR logging in
        session->recv_state = Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT;
      }
      else if ( session->buffer[0] == PKTIN_EF_ID )
      {
        // new seed since 6.0.5.0 (0xef should never appear in normal ipseed)
        if ( Plib::systemstate.config.verbose )
        {
          INFO_PRINTLN( "6.0.5.0+ Crypt Seed Message Received: Type {:#X}", (int)cstype );
        }
        session->recv_state = Network::ThreadedClient::RECV_STATE_CLIENTVERSION_WAIT;
      }
      else
      {
        session->cryptengine->Init( session->buffer, Crypt::CCryptBase::typeAuto );
        session->recv_state = Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT;
      }
    }
    // Else keep waiting for IP address.
  }
  if ( session->recv_state == Network::ThreadedClient::RECV_STATE_CLIENTVERSION_WAIT )
  {
    // receive and send to handler to get directly the version
    session->recv_remaining_nocrypt( 21 );
    if ( session->bytes_received == 21 )
    {
      session->recv_state = Network::ThreadedClient::RECV_STATE_MSGTYPE_WAIT;
      unsigned char tempseed[4];
      tempseed[0] = session->buffer[1];
      tempseed[1] = session->buffer[2];
      tempseed[2] = session->buffer[3];
      tempseed[3] = session->buffer[4];
      session->cryptengine->Init( tempseed, Crypt::CCryptBase::typeLogin );
      Network::PacketRegistry::handle_msg( PKTIN_EF_ID, &session->myClient, session->buffer );
    }
  }

  return false;
}

// TODO: We may want to take a buffer directly here instead of a ThreadedClient
bool check_inactivity( Network::ThreadedClient* session )
{
  switch ( session->buffer[0] )
  {
  case PKTBI_73_ID:
  // Fallthrough
  case PKTIN_09_ID:
  // Fallthrough
  case PKTBI_D6_IN_ID:
    return true;
  case PKTBI_BF_ID:
    if ( ( session->buffer[3] == 0 ) && ( session->buffer[4] == PKTBI_BF::TYPE_SESPAM ) )
      return true;
    break;
  default:
    return false;
  }

  return false;
}

// Something to consider: this could become Client::report_weird_packet(char* buffer, int
// bytes_received, const std::string& why)
void report_weird_packet( Network::ThreadedClient* session, const std::string& why )
{
  std::string tmp = fmt::format(
      "Client#{}: {} type {:#X}, {} bytes (IP: {}, Account: {})\n", session->myClient.instance_,
      why, (int)session->buffer[0], session->bytes_received, session->ipaddrAsString(),
      ( session->myClient.acct != nullptr ) ? session->myClient.acct->name() : "None" );

  if ( session->bytes_received <= 64 )
  {
    Clib::fdump( std::back_inserter( tmp ), session->buffer, session->bytes_received );
    POLLOG_INFO( tmp );
  }
  else
  {
    INFO_PRINT( tmp );
    Clib::fdump( std::back_inserter( tmp ), session->buffer, session->bytes_received );
    POLLOGLN( tmp );
  }
}

// Called when a packet size is registered but the
// packet has no handler in the core
void handle_unknown_packet( Network::ThreadedClient* session )
{
  if ( Plib::systemstate.config.display_unknown_packets )
    report_weird_packet( session, "Unknown packet" );
}

// Called when POL receives an undefined packet.
// Those have no registered size, so we must guess.
void handle_undefined_packet( Network::ThreadedClient* session )
{
  int msgtype = (int)session->buffer[0];
  INFO_PRINTLN( "Undefined message type {:#X}", msgtype );

  // Tries to read as much of it out as possible
  session->recv_remaining( sizeof session->buffer / 2 );

  report_weird_packet( session, "Unexpected message" );
}

// Handles variable-sized packets whose declared size is much larger than
// the receive buffer. This packet is most likely a client error, because
// the buffer should be big enough to handle anything sent by the known
// clients.
void handle_humongous_packet( Network::ThreadedClient* session, unsigned int reported_size )
{
  // Tries to read as much of it out as possible
  // (the client will be disconnected, but this may
  // be useful for debugging)
  session->recv_remaining( sizeof session->buffer / 2 );

  report_weird_packet( session, fmt::format( "Humongous packet (length {})", reported_size ) );
}
}  // namespace Pol::Core

namespace Pol::Network
{
// on_close determines how long to wait until on_logoff is called. An alternative would be to call
// test_logoff directly in the threadedclient_io_finalize.
int Client::on_close()
{
  unregister();
  INFO_PRINTLN( "Client disconnected from {} ({}/{} connections)", ipaddrAsString(),
                Core::networkManager.clients.size(),
                Core::networkManager.getNumberOfLoginClients() );

  Core::CoreSetSysTrayToolTip(
      Clib::tostring( Core::networkManager.clients.size() ) + " clients connected",
      Core::ToolTipPrioritySystem );

  if ( chr )
  {
    chr->disconnect_cleanup();
    gd->clear();
    chr->connected( false );
  }

  return test_logoff();
}

int Client::test_logoff()
{
  if ( !chr )
    return 0;

  int seconds_wait = 0;
  Core::ScriptDef sd;
  sd.quickconfig( "scripts/misc/logofftest.ecl" );
  if ( sd.exists() )
  {
    Bscript::BObject bobj( run_script_to_completion( sd, new Module::ECharacterRefObjImp( chr ) ) );
    if ( bobj.isa( Bscript::BObjectImp::OTLong ) )
    {
      const Bscript::BLong* blong = static_cast<const Bscript::BLong*>( bobj.impptr() );
      seconds_wait = blong->value();
    }
  }
  return seconds_wait;
}

void Client::on_logoff()
{
  if ( chr )
  {
    call_chr_scripts( chr, "scripts/misc/logoff.ecl", "logoff.ecl" );
    if ( chr->realm() )
    {
      chr->realm()->notify_left( *chr );
    }
  }
}

void Client::warn_idle()
{
  Network::PktHelper::PacketOut<Network::PktOut_53> msg;
  msg->Write<u8>( PKTOUT_53_WARN_CHARACTER_IDLE );
  msg.Send( this );

  if ( pause_count )
    restart();
}

bool Client::should_check_idle()
{
  return ( !chr || chr->cmdlevel() < Plib::systemstate.config.min_cmdlvl_ignore_inactivity ) &&
         Plib::systemstate.config.inactivity_warning_timeout &&
         Plib::systemstate.config.inactivity_disconnect_timeout && !disable_inactivity_timeout();
}

void Client::handle_msg( unsigned char* pktbuffer, int pktlen )
{
  const unsigned char msgtype = pktbuffer[0];
  try
  {
    INFO_PRINTLN_TRACE( 10 )( "Client#{}: message {:#x}", instance_, msgtype );

    // TODO: use PacketRegistry::handle_msg(...) ?
    MSG_HANDLER packetHandler = Network::PacketRegistry::find_handler( msgtype, this );
    passert( packetHandler.msglen != 0 );
    packetHandler.func( this, pktbuffer );
    Core::restart_all_clients();
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERRORLN( "Client#{}: Exception in message handler {:#X}: {}", instance_, (int)msgtype,
                    ex.what() );

    std::string tmp;
    Clib::fdump( std::back_inserter( tmp ), pktbuffer, pktlen );
    POLLOGLN( tmp );

    Core::restart_all_clients();
    throw;
  }
}

void Client::send_KR_encryption_response()
{
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
  msg.Send( this );
}


//


// ThreadedClient stuff
void ThreadedClient::process_delayed_packets()
{
  PacketThrottler pkt = myClient.movementqueue.front();

  if ( myClient.SpeedHackPrevention( false ) )
  {
    if ( isReallyConnected() )
    {
      myClient.handle_msg( pkt.pktbuffer, sizeof pkt.pktbuffer );
    }
    myClient.movementqueue.pop();
  }
}

}  // namespace Pol::Network
