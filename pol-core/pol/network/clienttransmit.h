#ifndef CLIENTSEND_H
#define CLIENTSEND_H

#include "../../clib/rawtypes.h"
#include "../../clib/message_queue.h"

#include <boost/noncopyable.hpp>

#include <memory>
#include <mutex>
#include <vector>

namespace Pol
{
namespace Network
{
class Client;

struct TransmitData
{
  Client* client;
  int len;
  std::vector<u8> data;
  bool disconnects;

  TransmitData() : client( nullptr ), len( 0 ), disconnects( false ){};
};

typedef std::unique_ptr<TransmitData> TransmitDataSPtr;
typedef Clib::message_queue<TransmitDataSPtr> ClientTransmitQueue;

class ClientTransmit : boost::noncopyable
{
public:
  ClientTransmit();
  ~ClientTransmit();

  void AddToQueue( Client* client, const void* data, int len );
  void QueueDisconnection( Client* client );
  void Cancel();

  TransmitDataSPtr NextQueueEntry();

private:
  ClientTransmitQueue _transmitqueue;
};

void ClientTransmitThread();
}
}
#endif
