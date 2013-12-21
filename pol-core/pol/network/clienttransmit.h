#ifndef CLIENTSEND_H
#define CLIENTSEND_H
#include "../../clib/stl_inc.h"
#include "../../clib/singleton.h"
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
	  ClientTransmit();
	  ~ClientTransmit();

	  void AddToQueue( Client* client, const void *data, int len );
	  void QueueDisconnection( Client* client );
	  void Cancel();

	  TransmitDataSPtr NextQueueEntry();
	private:
	  ClientTransmitQueue _transmitqueue;
	};

	typedef Clib::Singleton<ClientTransmit> ClientTransmitSingleton;

	void ClientTransmitThread();
  }
#define ADDTOSENDQUEUE(_client,_data,_len) Network::ClientTransmitSingleton::instance()->AddToQueue(_client,_data,_len)
}
#endif
