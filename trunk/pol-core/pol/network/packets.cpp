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
	packets.insert(PacketQueuePair(PKTBI_2C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2D_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2E_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_2F_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_3A_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_3C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_4F_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_53_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_54_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_55_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_56_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_65_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_66_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_6C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_6D_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_6E_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_6F_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_70_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_72_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_74_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_76_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_77_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_78_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_7C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_82_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_88_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_89_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_8C_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_90_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_93_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_95_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_98_OUT_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_99_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTBI_9A_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_9E_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A1_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A2_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A3_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A5_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A6_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A8_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_A9_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_AA_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_AB_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_AE_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_AF_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_B0_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_B7_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_B9_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_BA_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_BC_ID,new PacketQueueSingle()));

	packets.insert(PacketQueuePair(PKTBI_BF_ID,new PacketQueueSubs()));

	packets.insert(PacketQueuePair(PKTOUT_C1_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_C7_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_CC_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_DC_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_DD_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_E3_ID,new PacketQueueSingle()));
	packets.insert(PacketQueuePair(PKTOUT_F3_ID,new PacketQueueSingle()));
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
	u16 sub=pkt->getSubID();
	//critical start
	PolLock lck;
	PacketInterfaceQueueMap::iterator itr = packets.find(sub);
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
		packets.insert(PacketInterfaceQueuePair(sub,qu));
	}
	//critical end
}

int PacketQueueSubs::Count()
{
	int count=0;
	for ( PacketInterfaceQueueMap::iterator it=packets.begin() ; it != packets.end(); ++it )
	{
		count += it->second.size();
	}
	return count;
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
		case PKTBI_22_APPROVED_ID: return new PktOut_22();
		case PKTOUT_24_ID: return new PktOut_24();
		case PKTOUT_25_ID: return new PktOut_25();
		case PKTOUT_27_ID: return new PktOut_27();
		case PKTOUT_29_ID: return new PktOut_29();
		case PKTBI_2C_ID:  return new PktOut_2C();
		case PKTOUT_2D_ID: return new PktOut_2D();
		case PKTOUT_2E_ID: return new PktOut_2E();
		case PKTOUT_2F_ID: return new PktOut_2F();
		case PKTBI_3A_ID: return new PktOut_3A();
		case PKTOUT_3C_ID: return new PktOut_3C();
		case PKTOUT_4F_ID: return new PktOut_4F();
		case PKTOUT_53_ID: return new PktOut_53();
		case PKTOUT_54_ID: return new PktOut_54();
		case PKTOUT_55_ID: return new PktOut_55();
		case PKTBI_56_ID: return new PktOut_56();
		case PKTOUT_65_ID: return new PktOut_65();
		case PKTBI_66_ID: return new PktOut_66();
		case PKTBI_6C_ID: return new PktOut_6C();
		case PKTOUT_6D_ID: return new PktOut_6D();
		case PKTOUT_6E_ID: return new PktOut_6E();
		case PKTBI_6F_ID: return new PktOut_6F();
		case PKTOUT_70_ID: return new PktOut_70();
		case PKTBI_72_ID: return new PktOut_72();
		case PKTOUT_74_ID: return new PktOut_74();
		case PKTOUT_76_ID: return new PktOut_76();
		case PKTOUT_77_ID: return new PktOut_77();
		case PKTOUT_78_ID: return new PktOut_78();
		case PKTOUT_7C_ID: return new PktOut_7C();
		case PKTOUT_82_ID: return new PktOut_82();
		case PKTOUT_88_ID: return new PktOut_88();
		case PKTOUT_89_ID: return new PktOut_89();
		case PKTOUT_8C_ID: return new PktOut_8C();
		case PKTOUT_90_ID: return new PktOut_90();
		case PKTBI_93_ID: return new PktOut_93();
		case PKTBI_95_ID: return new PktOut_95();
		case PKTBI_98_OUT_ID: return new PktOut_98();
		case PKTBI_99_ID: return new PktOut_99();
		case PKTBI_9A_ID: return new PktOut_9A();
		case PKTOUT_9E_ID: return new PktOut_9E();
		case PKTOUT_A1_ID: return new PktOut_A1();
		case PKTOUT_A2_ID: return new PktOut_A2();
		case PKTOUT_A3_ID: return new PktOut_A3();
		case PKTOUT_A5_ID: return new PktOut_A5();
		case PKTOUT_A6_ID: return new PktOut_A6();
		case PKTOUT_A8_ID: return new PktOut_A8();
		case PKTOUT_A9_ID: return new PktOut_A9();
		case PKTOUT_AA_ID: return new PktOut_AA();
		case PKTOUT_AB_ID: return new PktOut_AB();
		case PKTOUT_AE_ID: return new PktOut_AE();
		case PKTOUT_AF_ID: return new PktOut_AF();
		case PKTOUT_B0_ID: return new PktOut_B0();
		case PKTOUT_B7_ID: return new PktOut_B7();
		case PKTOUT_B9_ID: return new PktOut_B9();
		case PKTOUT_BA_ID: return new PktOut_BA();
		case PKTOUT_BC_ID: return new PktOut_BC();

		case PKTBI_BF_ID:
		{
			switch (sub)
			{
				case PKTBI_BF::TYPE_CLOSE_GUMP:					return new PktOut_BF_Sub4();
				case PKTBI_BF::TYPE_CURSOR_HUE:					return new PktOut_BF_Sub6();
				case PKTBI_BF::TYPE_OBJECT_CACHE:				return new PktOut_BF_Sub10();
				case PKTBI_BF::TYPE_CLOSE_WINDOW:				return new PktOut_BF_Sub16();
				case PKTBI_BF::TYPE_ENABLE_MAP_DIFFS:			return new PktOut_BF_Sub18();
				case PKTBI_BF::TYPE_EXTENDED_STATS_OUT:			return new PktOut_BF_Sub19();
				case PKTBI_BF::TYPE_NEW_SPELLBOOK:				return new PktOut_BF_Sub1B();
				case PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT:			return new PktOut_BF_Sub1D();
				case PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL: return new PktOut_BF_Sub20();
				case PKTBI_BF::TYPE_DAMAGE:						return new PktOut_BF_Sub22();
				case PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER:		return new PktOut_BF_Sub2A();
				default: return NULL;
			}
		}
		case PKTOUT_C1_ID: return new PktOut_C1();
		case PKTOUT_C7_ID: return new PktOut_C7();
		case PKTOUT_CC_ID: return new PktOut_CC();
		case PKTOUT_DC_ID: return new PktOut_DC();
		case PKTOUT_DD_ID: return new PktOut_DD();
		case PKTOUT_E3_ID: return new PktOut_E3();
		case PKTOUT_F3_ID: return new PktOut_F3();
		default: return NULL;
	}
}
