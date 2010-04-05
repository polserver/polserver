/*
History
=======


Notes
=======
  Central class that holds every pkt object, this way client doesnt need to create pkts, just requests a object and
  after sending readds it
  New packet add: 
    PacketsSingleton::PacketsSingleton() add queue for it
	PacketInterface* GetPacket(u8 id, u16 sub) add pkt create

*/
#include "../../clib/stl_inc.h"

#include "../../clib/rawtypes.h"
#include "../../clib/logfile.h"
#include "../polsem.h"
#include "packets.h"

PacketsSingleton::PacketsSingleton()
{
	//insert packet queues at first creation
	packets.insert(PacketQueuePair(PKTOUT_0B_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_11_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_17_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_1A_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_1B_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_1C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_1D_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_20_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_21_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_22_APPROVED_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_24_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_25_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_27_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_29_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2D_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2E_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2F_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_3C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_4F_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_53_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_54_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_55_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_65_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_6D_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_6E_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_70_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_74_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_76_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_77_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_78_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_7C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_82_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_86_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_89_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_8C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_90_ID,new PacketQueueSingle()));

	packets.insert(PacketQueuePair(PKTOUT_AE_ID,new PacketQueueSingle()));

	packets.insert(PacketQueuePair(PKTBI_BF_ID,new PacketQueueSubs()));
}

PacketInterface* PacketsSingleton::getPacket(u8 id, u16 sub)
{
	PacketQueueMap::iterator itr = packets.find(id);
	if (itr != packets.end())
		return itr->second->GetNext(id, sub);
	return NULL;
}

void PacketsSingleton::ReAddPacket(PacketInterface* pkt)
{
	PacketQueueMap::iterator itr = packets.find(pkt->getID());
	if (itr != packets.end())
		itr->second->Add(pkt);
	else
		delete pkt;
}

void PacketsSingleton::LogStatus()
{
	Log("Packet status:\n");
	for ( PacketQueueMap::iterator it=packets.begin() ; it != packets.end(); ++it )
	{
		Log("0x%X :\n",it->first);
		it->second->LogStatus();
	}
	Log("Packet status end\n");
}

PacketInterface* PacketQueueSingle::GetNext(u8 id, u16 sub)
{
	//critical start
	PolLock lck;
	if (!packets.empty()) 
	{
		PacketInterface* pkt = packets.front(); // get next one
		packets.pop(); // and remove it from queue
		pkt->ReSetBuffer();
		return pkt;
	}
	else 
		return GetPacket(id);
	//critical end
}

void PacketQueueSingle::Add(PacketInterface* pkt)
{
	if (packets.size() > MAX_PACKETS_INSTANCES) // enough?
		delete pkt;
	else
	{
		//critical start
		PolLock lck;
		packets.push(pkt); // readd it
		//critical end
	}
}

void PacketQueueSingle::LogStatus()
{
	Log("  %d\n",packets.size());
}

PacketInterface* PacketQueueSubs::GetNext(u8 id, u16 sub)
{
	//critical start
	PolLock lck;
	if (!packets.empty()) 
	{
		PacketInterfaceQueueMap::iterator itr = packets.find(sub);
		if (itr != packets.end())
		{
			PacketInterface* pkt = itr->second.front(); // get next one
			itr->second.pop();  // and remove it from queue
			pkt->ReSetBuffer();
			return pkt;
		}
		else
			return GetPacket(id,sub);
	}
	else 
		return GetPacket(id,sub);
	//critical end
}

void PacketQueueSubs::Add(PacketInterface* pkt)
{
	//critical start
	PolLock lck;
	PacketInterfaceQueueMap::iterator itr = packets.find(pkt->getSubID());
	if (itr != packets.end())
	{
		if (itr->second.size() > MAX_PACKETS_INSTANCES) // enough?
			delete pkt;
		else
			itr->second.push(pkt); // readd it
	}
	else // new sub
	{
		PacketInterfaceQueue qu;
		qu.push(pkt);
		packets.insert(PacketInterfaceQueuePair(pkt->getSubID(),qu));
	}
	//critical end
}

void PacketQueueSubs::LogStatus()
{
	for ( PacketInterfaceQueueMap::iterator it=packets.begin() ; it != packets.end(); ++it )
	{
		Log("  0x%X :\n",it->first);
		Log("    %d\n",it->second.size());
	}
}

// creates new packets
PacketInterface* GetPacket(u8 id, u16 sub)
{
	switch (id)
	{
		case PKTOUT_0B_ID: return new PktOut_0B();
		case PKTOUT_11_ID: return new PktOut_11();
		case PKTOUT_17_ID: return new PktOut_17();
		case PKTOUT_1A_ID: return new PktOut_1A();
		case PKTOUT_1B_ID: return new PktOut_1B();
		case PKTOUT_1C_ID: return new PktOut_1C();
		case PKTOUT_1D_ID: return new PktOut_1D();
		case PKTOUT_20_ID: return new PktOut_20();
		case PKTOUT_21_ID: return new PktOut_21();
		case PKTBI_22_APPROVED_ID: return new PktBi_22();
		case PKTOUT_24_ID: return new PktOut_24();
		case PKTOUT_25_ID: return new PktOut_25();
		case PKTOUT_27_ID: return new PktOut_27();
		case PKTOUT_29_ID: return new PktOut_29();
		case PKTOUT_2D_ID: return new PktOut_2D();
		case PKTOUT_2E_ID: return new PktOut_2E();
		case PKTOUT_2F_ID: return new PktOut_2F();
		case PKTOUT_3C_ID: return new PktOut_3C();
		case PKTOUT_4F_ID: return new PktOut_4F();
		case PKTOUT_53_ID: return new PktOut_53();
		case PKTOUT_54_ID: return new PktOut_54();
		case PKTOUT_55_ID: return new PktOut_55();
		case PKTOUT_65_ID: return new PktOut_65();
		case PKTOUT_6D_ID: return new PktOut_6D();
		case PKTOUT_6E_ID: return new PktOut_6E();
		case PKTOUT_70_ID: return new PktOut_70();
		case PKTOUT_74_ID: return new PktOut_74();
		case PKTOUT_76_ID: return new PktOut_76();
		case PKTOUT_77_ID: return new PktOut_77();
		case PKTOUT_78_ID: return new PktOut_78();
		case PKTOUT_7C_ID: return new PktOut_7C();
		case PKTOUT_82_ID: return new PktOut_82();
		case PKTOUT_86_ID: return new PktOut_86();
		case PKTOUT_89_ID: return new PktOut_89();
		case PKTOUT_8C_ID: return new PktOut_8C();
		case PKTOUT_90_ID: return new PktOut_90();

		case PKTOUT_AE_ID: return new PktOut_AE();
		case PKTBI_BF_ID:
		{
			switch (sub)
			{
				case 0x04: return new Pktout_bf_sub4_closegump();
				default: return NULL;
			}
		}
		default: return NULL;
	}
}
