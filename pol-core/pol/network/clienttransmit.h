#ifndef CLIENTSEND_H
#define CLIENTSEND_H

#include <memory>
#include <mutex>
#include "../../clib/stl_inc.h"
#include "../../clib/rawtypes.h"

#include "../../clib/message_queue.h"

#include <boost/noncopyable.hpp>
namespace Pol {
  namespace Network {
	class Client;

	struct TransmitData
	{
	  Client* client;
	  int len;
	  vector<u8> data;
	  bool disconnects;

	  TransmitData() : client( nullptr ), len( 0 ), disconnects( false ) {};
	};

	typedef std::unique_ptr<TransmitData> TransmitDataSPtr;
	typedef Clib::message_queue<TransmitDataSPtr> ClientTransmitQueue;

	class ClientTransmit : boost::noncopyable
	{
     public:
      static ClientTransmit& get();

     public:
      ~ClientTransmit();

      void AddToQueue(Client* client, const void* data, int len);
      void QueueDisconnection(Client* client);
      void Cancel();

      TransmitDataSPtr NextQueueEntry();

     private:
      ClientTransmit();
      static std::unique_ptr<ClientTransmit> _instance;
      static std::once_flag _onceFlag;
      ClientTransmitQueue _transmitqueue;
    };

    typedef ClientTransmit ClientTransmitSingleton;

    void ClientTransmitThread();
  }
#define ADDTOSENDQUEUE(_client, _data, _len) Network::ClientTransmitSingleton::get().AddToQueue(_client, _data, _len)
}
#endif
