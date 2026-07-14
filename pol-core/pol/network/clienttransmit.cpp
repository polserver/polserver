#include "clienttransmit.h"

#include "../../clib/esignal.h"
#include "../../clib/rawtypes.h"
#include "../globals/network.h"
#include "../polsem.h"
#include "client.h"


namespace Pol::Network
{
void ClientTransmit::Cancel()
{
  _transmitqueue.cancel();
}

void ClientTransmit::AddToQueue( Client* client, const void* data, int len )
{
  const u8* message = static_cast<const u8*>( data );
  _transmitqueue.push_move( { .client = client->getWeakPtr(),
                              .data = { message, message + len },
                              .disconnects = false } );
}

void ClientTransmit::QueueDisconnection( Client* client )
{
  _transmitqueue.push_move( { .client = client->getWeakPtr(), .disconnects = true } );
}

void ClientTransmit::QueueDelete( Client* client )
{
  _transmitqueue.push_move( { .client = client->getWeakPtr(), .remove = true } );
}

ClientTransmit::TransmitData ClientTransmit::NextQueueEntry()
{
  TransmitData transmitdata;
  _transmitqueue.pop_wait( &transmitdata );
  return transmitdata;
}

void ClientTransmitThread()
{
  ClientTransmit* transmit_instance = Core::networkManager.clientTransmit.get();
  while ( !Clib::exit_signalled )
  {
    try
    {
      auto data = transmit_instance->NextQueueEntry();
      if ( !data.client.exists() )
        continue;

      if ( data.remove )
      {
        Core::PolLock lock;
        delete data.client.get_weakptr();
      }
      else if ( data.disconnects )
      {
        data.client->forceDisconnect();
      }
      else if ( data.client->isReallyConnected() )
      {
        data.client->transmit( static_cast<void*>( &data.data[0] ),
                               static_cast<int>( data.data.size() ) );
      }
    }
    catch ( ClientTransmit::ClientTransmitQueue::Canceled& )
    {
      return;
    }
  }
}
}  // namespace Pol::Network
