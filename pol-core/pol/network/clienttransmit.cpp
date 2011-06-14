#include "clientTransmit.h"
#include "client.h"
#include "../../clib/esignal.h"
#include "../polsem.h"

ClientTransmit::ClientTransmit()
{

}

ClientTransmit::~ClientTransmit()
{

}

void ClientTransmit::AddToQueue(Client* client, const void *data, int len)
{
	const u8* message = static_cast<const u8*>(data);
	TransmitData transmitdata;
	transmitdata.client=client;
	transmitdata.len=len;
	transmitdata.data.assign(message,message+len);
	_TransmitQueueMutex.lock();
	_transmitqueue.push(transmitdata);
	_TransmitQueueMutex.unlock();
	send_ClientTransmit_pulse();
}

bool ClientTransmit::HasQueueEntries()
{
	_TransmitQueueMutex.lock();
	bool ret = !_transmitqueue.empty();
	_TransmitQueueMutex.unlock();
	return ret;
}

TransmitData ClientTransmit::NextQueueEntry()
{
	TransmitData data;
	_TransmitQueueMutex.lock();
	if (!_transmitqueue.empty())
	{
		data = _transmitqueue.front();
		_transmitqueue.pop();
	}
	_TransmitQueueMutex.unlock();
	return data;
}

void ClientTransmitThread()
{
	ClientTransmit* clienttransmit = ClientTransmitSingleton::instance();
	while (!exit_signalled)
	{
		while (clienttransmit->HasQueueEntries())
		{
			TransmitData data = clienttransmit->NextQueueEntry();
			if (data.client != NULL)
			{
				if (!data.client->disconnect)
					data.client->transmit(static_cast<void*>(&data.data[0]),data.len);
			}
		}
		wait_for_ClientTransmit_pulse(1000);
	}
}
