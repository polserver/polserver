#ifndef CLIENTSEND_H
#define CLIENTSEND_H
#include "../../clib/stl_inc.h"
#include "../../clib/singleton.h"
#include "../../clib/hbmutex.h"
#include "../../clib/rawtypes.h"

class Client;

struct TransmitData
{
	Client* client;
	int len;
	vector<u8> data;
	
};
typedef queue<TransmitData> ClientTransmitQueue;

class ClientTransmit
{
public:
	ClientTransmit();
	~ClientTransmit();
	void AddToQueue(Client* client, const void *data, int len);
	bool HasQueueEntries();
	TransmitData NextQueueEntry();


private:
	ClientTransmitQueue _transmitqueue;
	Mutex _TransmitQueueMutex;
};



typedef Singleton<ClientTransmit> ClientTransmitSingleton;
#define ADDTOSENDQUEUE(_client,_data,_len) ClientTransmitSingleton::instance()->AddToQueue(_client,_data,_len)

void ClientTransmitThread();
#endif
