#include "clienttransmit.h"
#include "client.h"
#include "../../clib/esignal.h"


ClientTransmit::ClientTransmit() : _transmitqueue()
{
}

ClientTransmit::~ClientTransmit()
{
}

void ClientTransmit::Cancel()
{
	_transmitqueue.cancel();
}
void ClientTransmit::AddToQueue(Client* client, const void *data, int len)
{
	const u8* message = static_cast<const u8*>(data);
	auto transmitdata = TransmitDataSPtr(new TransmitData);
	transmitdata->client=client;
	transmitdata->len=len;
	transmitdata->data.assign(message,message+len);
	transmitdata->disconnects = false;
	_transmitqueue.push_move(transmitdata);
}

void ClientTransmit::QueueDisconnection(Client* client)
{
	auto transmitdata = TransmitDataSPtr(new TransmitData);
	transmitdata->disconnects = true;
	transmitdata->client = client;
	_transmitqueue.push_move(transmitdata);
}

TransmitDataSPtr ClientTransmit::NextQueueEntry()
{
	auto transmitdata = TransmitDataSPtr();
	_transmitqueue.pop_wait(transmitdata);
	return transmitdata;
}

void ClientTransmitThread()
{
	ClientTransmit* clienttransmit = ClientTransmitSingleton::instance();
	while (!exit_signalled)
	{
		try
		{
			auto data = clienttransmit->NextQueueEntry();
			if (data->client != nullptr)
			{
				if (data->disconnects)
					data->client->forceDisconnect();
				else if (data->client->isReallyConnected())
					data->client->transmit(static_cast<void*>(&data->data[0]),data->len, true);
			}
		}
		catch (ClientTransmitQueue::Canceled& e)
		{
			return;
		}
	}
}
