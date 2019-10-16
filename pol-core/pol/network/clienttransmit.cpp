#include "clienttransmit.h"

#include "../../clib/esignal.h"
#include "../../clib/rawtypes.h"
#include "../globals/network.h"
#include "../globals/worldthread.h"
#include "../polsem.h"
#include "client.h"

namespace Pol
{
namespace Network
{
ClientTransmit::ClientTransmit() : _transmitqueue() {}

ClientTransmit::~ClientTransmit() {}

void ClientTransmit::Cancel()
{
  _transmitqueue.cancel();
}
void ClientTransmit::AddToQueue( Client* client, const void* data, int len )
{
  const u8* message = static_cast<const u8*>( data );
  auto transmitdata = TransmitDataSPtr( new TransmitData );
  transmitdata->client = client->getWeakPtr();
  transmitdata->len = len;
  transmitdata->data.assign( message, message + len );
  transmitdata->disconnects = false;
  _transmitqueue.push_move( std::move( transmitdata ) );
}

void ClientTransmit::QueueDisconnection( Client* client )
{
  auto transmitdata = TransmitDataSPtr( new TransmitData );
  transmitdata->disconnects = true;
  transmitdata->client = client->getWeakPtr();
  _transmitqueue.push_move( std::move( transmitdata ) );
}

void ClientTransmit::QueueDelete( Client* client )
{
  auto transmitdata = TransmitDataSPtr( new TransmitData );
  transmitdata->remove = true;
  transmitdata->client = client->getWeakPtr();
  _transmitqueue.push_move( std::move( transmitdata ) );
}

TransmitDataSPtr ClientTransmit::NextQueueEntry()
{
  auto transmitdata = TransmitDataSPtr();
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
      if ( data->client.exists() )
      {
        if ( data->remove )
          Core::WorldThread::request( [&] { Client::Delete( data->client.get_weakptr() ); } ).get();
        else if ( data->disconnects )
        {
          data->client->forceDisconnect();
        }
        else if ( data->client->isReallyConnected() )
          data->client->transmit( static_cast<void*>( &data->data[0] ), data->len, true );
      }
    }
    catch ( ClientTransmitQueue::Canceled& )
    {
      return;
    }
  }
}
}  // namespace Network
}  // namespace Pol
