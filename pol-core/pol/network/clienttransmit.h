#ifndef CLIENTSEND_H
#define CLIENTSEND_H

#include <memory>
#include <mutex>
#include <vector>

#include "../../clib/message_queue.h"
#include "../../clib/rawtypes.h"
#include "../../clib/weakptr.h"

namespace Pol
{
namespace Network
{
class Client;

struct TransmitData
{
  // store a weak_ptr as a guard for pkts after deleting
  weak_ptr<Client> client;
  int len;
  std::vector<u8> data;
  bool disconnects;
  bool remove;

  TransmitData() : client( 0 ), len( 0 ), disconnects( false ), remove( false ){};
};

using TransmitDataSPtr = std::unique_ptr<TransmitData>;
using ClientTransmitQueue = Clib::message_queue<TransmitDataSPtr>;

class ClientTransmit
{
public:
  ClientTransmit();
  ~ClientTransmit();
  ClientTransmit( const ClientTransmit& ) = delete;
  ClientTransmit& operator=( const ClientTransmit& ) = delete;

  void AddToQueue( Client* client, const void* data, int len );
  void QueueDisconnection( Client* client );
  // queue delete and perform it in transmitthread, to be sure
  // that the weak_ptr stays valid without PolLock
  void QueueDelete( Client* client );
  void Cancel();

  TransmitDataSPtr NextQueueEntry();

private:
  ClientTransmitQueue _transmitqueue;
};

void ClientTransmitThread();
}  // namespace Network
}  // namespace Pol
#endif
