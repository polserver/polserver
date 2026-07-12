#pragma once

#include <vector>

#include "../../clib/message_queue.h"
#include "../../clib/rawtypes.h"
#include "../../clib/weakptr.h"


namespace Pol::Network
{
class Client;

void ClientTransmitThread();

class ClientTransmit
{
public:
  ClientTransmit() = default;
  ~ClientTransmit() = default;
  ClientTransmit( const ClientTransmit& ) = delete;
  ClientTransmit& operator=( const ClientTransmit& ) = delete;

  void AddToQueue( Client* client, const void* data, int len );
  void QueueDisconnection( Client* client );
  // queue delete and perform it in transmitthread, to be sure
  // that the weak_ptr stays valid without PolLock
  void QueueDelete( Client* client );
  void Cancel();

protected:
  friend void ClientTransmitThread();

  struct TransmitData
  {
    // store a weak_ptr as a guard for pkts after deleting
    weak_ptr<Client> client{ nullptr };
    std::vector<u8> data{};
    bool disconnects{ false };
    bool remove{ false };
  };

  using ClientTransmitQueue = Clib::message_queue<TransmitData>;
  TransmitData NextQueueEntry();

private:
  ClientTransmitQueue _transmitqueue{};
};

}  // namespace Pol::Network
