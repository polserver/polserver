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
	auto transmitdata = std::make_shared<TransmitData>();
	transmitdata->client=client;
	transmitdata->len=len;
	transmitdata->data.assign(message,message+len);
	transmitdata->disconnects = false;
	_transmitqueue.push(transmitdata);
}

void ClientTransmit::QueueDisconnection(Client* client)
{
	auto transmitdata = std::make_shared<TransmitData>();
	transmitdata->disconnects = true;
	transmitdata->client = client;
	_transmitqueue.push(transmitdata);
}

TransmitDataSPtr ClientTransmit::NextQueueEntry()
{
	auto transmitdata = std::make_shared<TransmitData>();
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
			if (data->client != NULL)
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
