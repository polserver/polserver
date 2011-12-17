#include "clienttransmit.h"
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
	transmitdata.disconnects = false;
	{
		LocalMutex guard(&_TransmitQueueMutex);
		_transmitqueue.push(transmitdata);
	}
	send_ClientTransmit_pulse();
}

void ClientTransmit::QueueDisconnection(Client* client)
{
	TransmitData transmitdata;
	transmitdata.disconnects = true;
	transmitdata.client = client;
	{
		LocalMutex guard(&_TransmitQueueMutex);
		_transmitqueue.push(transmitdata);
	}
	send_ClientTransmit_pulse();
}

bool ClientTransmit::HasQueueEntries()
{
	LocalMutex guard(&_TransmitQueueMutex);
	return !_transmitqueue.empty();
}

TransmitData ClientTransmit::NextQueueEntry()
{
	TransmitData data;
	LocalMutex guard(&_TransmitQueueMutex);
	if (!_transmitqueue.empty())
	{
		data = _transmitqueue.front();
		_transmitqueue.pop();
	}
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
				if (data.disconnects)
					data.client->forceDisconnect();
				else if (data.client->isReallyConnected())
					data.client->transmit(static_cast<void*>(&data.data[0]),data.len,true);
			}
		}
		wait_for_ClientTransmit_pulse(1000);
	}
}
