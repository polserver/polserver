#ifndef CLIENTSEND_H
#define CLIENTSEND_H
#include "../../clib/stl_inc.h"
#include "../../clib/singleton.h"
#include "../../clib/rawtypes.h"

#include "../../clib/message_queue.h"

#include <boost/noncopyable.hpp>

class Client;

struct TransmitData
{
	Client* client;
	int len;
	vector<u8> data;
	bool disconnects;
	
	TransmitData() : client(nullptr), len(0), disconnects(false) {};
};

typedef std::shared_ptr<TransmitData> TransmitDataSPtr;
typedef message_queue<TransmitDataSPtr> ClientTransmitQueue;

class ClientTransmit : boost::noncopyable
{
public:
	ClientTransmit();
	~ClientTransmit();

	void AddToQueue(Client* client, const void *data, int len);
	void QueueDisconnection(Client* client);
	void Cancel();

	TransmitDataSPtr NextQueueEntry();
private:
	ClientTransmitQueue _transmitqueue;
};



typedef Singleton<ClientTransmit> ClientTransmitSingleton;
#define ADDTOSENDQUEUE(_client,_data,_len) ClientTransmitSingleton::instance()->AddToQueue(_client,_data,_len)

void ClientTransmitThread();
#endif
