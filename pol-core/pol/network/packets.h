/*
History
=======

Notes
=======

*/

#ifndef POL_PACKETS_H
#define POL_PACKETS_H

#include <climits>
#include "../../clib/stl_inc.h"

#include "../../clib/endian.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../../clib/singleton.h"
#include "../../clib/strutil.h"
#include "../layers.h"
#include "../pktboth.h"
#include "../pktbothid.h"
#include "../pktdef.h"
#include "../pktoutid.h"
#include "../realms.h"
#include "../ucfg.h"

#define MAX_PACKETS_INSTANCES 100

//interface for packets
class PacketInterface
{
	public:
		PacketInterface(){};
		virtual ~PacketInterface(){};
		u16 offset;
		virtual void ReSetBuffer() {};
		virtual char* getBuffer() { return NULL; }
		virtual inline u8 getID() { return 0; }
		virtual inline u16 getSubID() { return 0; }
};

typedef queue<PacketInterface*> PacketInterfaceQueue;
typedef std::map<u16, PacketInterfaceQueue> PacketInterfaceQueueMap;
typedef pair<u16, PacketInterfaceQueue> PacketInterfaceQueuePair;

// interface for the two different types of packetqueues ("normal" packets and packets with subs)
class PacketQueue
{
	public:
		PacketQueue(){};
		virtual ~PacketQueue(){};
	public:
		virtual PacketInterface* GetNext(u8 id, u16 sub=0) {return NULL;}
		virtual void Add(PacketInterface* pkt){};
		virtual int Count(){ return 0; }
		virtual bool HasSubs(){ return false; }
		virtual PacketInterfaceQueueMap* GetSubs(){ return NULL; }
};

// "normal" packet queue
class PacketQueueSingle : public PacketQueue
{
	public:
		PacketQueueSingle(){};
		~PacketQueueSingle(){};
	private:
		PacketInterfaceQueue packets;
	public:
		PacketInterface* GetNext(u8 id, u16 sub=0);
		void Add(PacketInterface* pkt);
		int Count(){ return packets.size(); }
};

// packet with subs queue
class PacketQueueSubs : public PacketQueue
{
	public:
		PacketQueueSubs(){};
		~PacketQueueSubs(){};
	private:
		PacketInterfaceQueueMap packets;
	public:
		PacketInterface* GetNext(u8 id, u16 sub=0);
		void Add(PacketInterface* pkt);
		int Count();
		bool HasSubs(){ return true; }
		PacketInterfaceQueueMap* GetSubs(){ return &packets; }
};

typedef pair<u8, PacketQueue*> PacketQueuePair;
typedef std::map<u8, PacketQueue*> PacketQueueMap;

// singleton "holder" of packets !EntryPoint!
class PacketsSingleton
{
	public:
		PacketsSingleton();
		~PacketsSingleton(){};
	private:
		PacketQueueMap packets;
	public:
		PacketInterface* getPacket(u8 id, u16 sub=0);
		void ReAddPacket(PacketInterface* pkt);
		PacketQueueMap* getPackets() { return &packets; } 
};

// the real definition
typedef Singleton<PacketsSingleton> Packets;

//wierdo generic template definitions for packets

// "writer"class
template <u8 _id, u16 _size>
class PacketWriter : public PacketInterface
{
	public:
		char buffer[_size];
		char* getBuffer() { return &buffer[offset]; }
		inline u8 getID() { return buffer[0]; }

		void Write(u32 x)
		{
			passert_always_r(offset+4<=_size, "pkt "+hexint(_id));
			(*(u32*)&buffer[offset]) = x;
			offset += 4;
		};
		void Write(s32 x)
		{
			passert_always_r(offset+4<=_size, "pkt "+hexint(_id));
			(*(s32*)&buffer[offset]) = x;
			offset += 4;
		};
		void Write(u16 x)
		{
			passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
			(*(u16*)&buffer[offset]) = x;
			offset += 2;
		};
		void Write(s16 x)
		{
			passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
			(*(s16*)&buffer[offset]) = x;
			offset += 2;
		};
		void Write(u8 x) 
		{ 
			passert_always_r(offset+1<=_size, "pkt "+hexint(_id));
			buffer[offset++] = x;
		};
		void Write(s8 x)
		{
			passert_always_r(offset+1<=_size, "pkt "+hexint(_id));
			buffer[offset++] = x;
		};
		void WriteFlipped(u32 x)
		{
			passert_always_r(offset+4<=_size, "pkt "+hexint(_id));
			(*(u32*)&buffer[offset]) = cfBEu32(x);
			offset += 4;
		};
		void WriteFlipped(s32 x)
		{
			passert_always_r(offset+4<=_size, "pkt "+hexint(_id));
			(*(s32*)&buffer[offset]) = cfBEu32(x);
			offset += 4;
		};
		void WriteFlipped(u16 x)
		{
			passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
			(*(u16*)&buffer[offset]) = cfBEu16(x);
			offset += 2;
		};
		void WriteFlipped(s16 x)
		{
			passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
			(*(s16*)&buffer[offset]) = cfBEu16(x);
			offset += 2;
		};
		void Write(const char* x, u16 len, bool nullterm=true)
		{
			passert_always_r(offset+len<=_size, "pkt "+hexint(_id));
			strncpy(&buffer[offset], x, nullterm ? len-1 : len);
			offset += len;
		}
		void Write(u8 x[], u16 len)
		{
			passert_always_r(offset+len<=_size, "pkt "+hexint(_id));
			memcpy(&buffer[offset], x, len);
			offset += len;
		}
		void Write(const u16* x, u16 len, bool nullterm=true)
		{
			passert_always_r(offset+len*2<=_size, "pkt "+hexint(_id));
			u16* _buffer = ((u16*)&buffer[offset]);
			offset += len*2;
			while (len-- > 0)
			{
				*(_buffer++) = *x++;
			}
			if (nullterm)
			{
				passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
				offset += 2;
			}
		}
		void WriteFlipped(const u16* x, u16 len, bool nullterm=true)
		{
			passert_always_r(offset+len*2<=_size, "pkt "+hexint(_id));
			u16* _buffer = ((u16*)&buffer[offset]);
			offset += len*2;
			while (len-- > 0)
			{
				*(_buffer++) = ctBEu16(*x);
				++x;
			}
			if (nullterm)
			{
				passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
				offset += 2;
			}
		}
};

// "normal" pkt
template <u8 _id, u16 _size>
class PacketTemplate : public PacketWriter<_id, _size>
{
	public:
		PacketTemplate() { ReSetBuffer(); }
		void ReSetBuffer() { memset(buffer,0,_size); buffer[0]=_id; offset=1; }
};

// sub packet
template <u8 _id, u16 _suboff, u16 _sub, u16 _size>
class PacketTemplateSub : public PacketWriter<_id, _size>
{
	public:
		PacketTemplateSub() { ReSetBuffer(); }
		void ReSetBuffer() 
		{ 
			memset(buffer,0,_size);
			buffer[0]=_id;
			(*(u16*)&buffer[_suboff]) = cfBEu16(_sub);
			offset=1;
		}
		inline u16 getSubID() { return _sub; /*ctBEu16((*(u16*)&buffer[_suboff]));*/ }
};

//special def for encrypted buffer
template <u8 _id, u16 _size>
class EmptyBufferTemplate : public PacketInterface
{
public:
	EmptyBufferTemplate() { ReSetBuffer(); }
	char buffer[_size];
	void ReSetBuffer() 
	{ 
		memset(buffer,0,_size);
		offset=0;
	}
	char* getBuffer() { return &buffer[offset]; }
	inline u8 getID() { return _id; }
};

// creates new packets
PacketInterface* GetPacket(u8 id, u16 sub=0);

#define REQUESTPACKET(_pkt,_id) static_cast<_pkt*>(Packets::instance()->getPacket(_id))
#define REQUESTSUBPACKET(_pkt,_id,_sub) static_cast<_pkt*>(Packets::instance()->getPacket(_id,_sub))
#define READDPACKET(_msg) Packets::instance()->ReAddPacket(_msg)

// buffer for encrypted Data send with a dummy pktid
// NOTE: redefine id if pkt 0x0 ever gets send
#define ENCRYPTEDPKTBUFFER 0

// Packet defs start
typedef EmptyBufferTemplate<ENCRYPTEDPKTBUFFER,0xFFFF> EncryptedPktBuffer;

typedef PacketTemplate<PKTOUT_0B_ID,7> PktOut_0B;
typedef PacketTemplate<PKTOUT_11_ID,91> PktOut_11;
typedef PacketTemplate<PKTOUT_17_ID,12> PktOut_17;
typedef PacketTemplate<PKTOUT_1A_ID,20> PktOut_1A;
typedef PacketTemplate<PKTOUT_1B_ID,37> PktOut_1B;
typedef PacketTemplate<PKTOUT_1C_ID,44 + SPEECH_MAX_LEN + 1> PktOut_1C;
typedef PacketTemplate<PKTOUT_1D_ID,5> PktOut_1D;
typedef PacketTemplate<PKTOUT_20_ID,19> PktOut_20;
typedef PacketTemplate<PKTOUT_21_ID,8> PktOut_21;
typedef PacketTemplate<PKTBI_22_APPROVED_ID,3> PktOut_22;
typedef PacketTemplate<PKTOUT_24_ID,7> PktOut_24;
typedef PacketTemplate<PKTOUT_25_ID,21> PktOut_25;
typedef PacketTemplate<PKTOUT_27_ID,2> PktOut_27;
typedef PacketTemplate<PKTOUT_29_ID,1> PktOut_29;
typedef PacketTemplate<PKTBI_2C_ID,2> PktOut_2C;
typedef PacketTemplate<PKTOUT_2D_ID,17> PktOut_2D;
typedef PacketTemplate<PKTOUT_2E_ID,15> PktOut_2E;
typedef PacketTemplate<PKTOUT_2F_ID,10> PktOut_2F;
typedef PacketTemplate<PKTBI_3A_ID,6 + 100 * 9> PktOut_3A;
typedef PacketTemplate<PKTBI_3B_ID,8> PktOut_3B;
typedef PacketTemplate<PKTOUT_3C_ID,5 + MAX_CONTAINER_ITEMS * 20> PktOut_3C;
typedef PacketTemplate<PKTOUT_4F_ID,2> PktOut_4F;
typedef PacketTemplate<PKTOUT_53_ID,2> PktOut_53;
typedef PacketTemplate<PKTOUT_54_ID,12> PktOut_54;
typedef PacketTemplate<PKTOUT_55_ID,1> PktOut_55;
typedef PacketTemplate<PKTBI_56_ID,11> PktOut_56;
typedef PacketTemplate<PKTOUT_65_ID,4> PktOut_65;
typedef PacketTemplate<PKTBI_66_ID,0xFFFF> PktOut_66;
typedef PacketTemplate<PKTBI_6C_ID,19> PktOut_6C;
typedef PacketTemplate<PKTOUT_6D_ID,3> PktOut_6D;
typedef PacketTemplate<PKTOUT_6E_ID,14> PktOut_6E;
typedef PacketTemplate<PKTBI_6F_ID,47> PktOut_6F;
typedef PacketTemplate<PKTOUT_70_ID,28> PktOut_70;
typedef PacketTemplate<PKTBI_72_ID,5> PktOut_72;
typedef PacketTemplate<PKTOUT_74_ID,0xFFFF> PktOut_74;
typedef PacketTemplate<PKTOUT_76_ID,16> PktOut_76;
typedef PacketTemplate<PKTOUT_77_ID,17> PktOut_77;
typedef PacketTemplate<PKTOUT_78_ID,19 + (9 * HIGHEST_LAYER) + 4> PktOut_78;
typedef PacketTemplate<PKTOUT_7C_ID,2000> PktOut_7C;
typedef PacketTemplate<PKTOUT_82_ID,2> PktOut_82;
typedef PacketTemplate<PKTOUT_88_ID,66> PktOut_88;
typedef PacketTemplate<PKTOUT_89_ID,7+(5*(NUM_LAYERS+1))+1> PktOut_89;
typedef PacketTemplate<PKTOUT_8C_ID,11> PktOut_8C;
typedef PacketTemplate<PKTOUT_90_ID,19> PktOut_90;
typedef PacketTemplate<PKTBI_93_ID,99> PktOut_93;
typedef PacketTemplate<PKTBI_95_ID,9> PktOut_95;
typedef PacketTemplate<PKTBI_98_OUT_ID,37> PktOut_98;
typedef PacketTemplate<PKTBI_99_ID,26> PktOut_99;
typedef PacketTemplate<PKTBI_9A_ID,16> PktOut_9A;
typedef PacketTemplate<PKTOUT_9E_ID,0xFFFF> PktOut_9E;
typedef PacketTemplate<PKTOUT_A1_ID,9> PktOut_A1;
typedef PacketTemplate<PKTOUT_A2_ID,9> PktOut_A2;
typedef PacketTemplate<PKTOUT_A3_ID,9> PktOut_A3;
typedef PacketTemplate<PKTOUT_A5_ID,URL_MAX_LEN+4> PktOut_A5;
typedef PacketTemplate<PKTOUT_A6_ID,10010> PktOut_A6;
typedef PacketTemplate<PKTOUT_A8_ID,2000> PktOut_A8;
typedef PacketTemplate<PKTOUT_A9_ID,10000> PktOut_A9;
typedef PacketTemplate<PKTOUT_AA_ID,5> PktOut_AA;
typedef PacketTemplate<PKTOUT_AB_ID,531> PktOut_AB;
typedef PacketTemplate<PKTOUT_AE_ID,(SPEECH_MAX_LEN + 1)*2+48> PktOut_AE;
typedef PacketTemplate<PKTOUT_AF_ID,13> PktOut_AF;
typedef PacketTemplate<PKTOUT_B0_ID,0xFFFF> PktOut_B0;
typedef PacketTemplate<PKTOUT_B7_ID,521> PktOut_B7;
typedef PacketTemplate<PKTBI_B8_OUT_ID,7+5*(SPEECH_MAX_LEN+1)> PktOut_B8;
typedef PacketTemplate<PKTOUT_B9_ID,5> PktOut_B9;
typedef PacketTemplate<PKTOUT_BA_ID,6> PktOut_BA;
typedef PacketTemplate<PKTOUT_BC_ID,3> PktOut_BC;

typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CLOSE_GUMP,5+8> PktOut_BF_Sub4;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_PARTY_SYSTEM,0xFFFF> PktOut_BF_Sub6;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CURSOR_HUE,5+1> PktOut_BF_Sub8;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_OBJECT_CACHE,5+8> PktOut_BF_Sub10;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CLOSE_WINDOW,5+8> PktOut_BF_Sub16;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_ENABLE_MAP_DIFFS,5+4+MAX_NUMER_REALMS*8> PktOut_BF_Sub18;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_EXTENDED_STATS_OUT,5+7> PktOut_BF_Sub19;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_NEW_SPELLBOOK,5+18> PktOut_BF_Sub1B;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT,5+8> PktOut_BF_Sub1D;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL,5+12> PktOut_BF_Sub20;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_DAMAGE,5+6> PktOut_BF_Sub22;
typedef PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER,5+2> PktOut_BF_Sub2A;

typedef PacketTemplate<PKTOUT_C1_ID,48+(SPEECH_MAX_LEN+1)+2> PktOut_C1;
typedef PacketTemplate<PKTBI_C2_ID,21> PktOut_C2;
typedef PacketTemplate<PKTOUT_C7_ID,49> PktOut_C7;
typedef PacketTemplate<PKTOUT_CC_ID,49+(SPEECH_MAX_LEN+1)*2+SPEECH_MAX_LEN+1> PktOut_CC;
typedef PacketTemplate<PKTBI_D6_OUT_ID,25+(4*SPEECH_MAX_LEN)> PktOut_D6;
typedef PacketTemplate<PKTOUT_DC_ID,9> PktOut_DC;
typedef PacketTemplate<PKTOUT_DD_ID,0xFFFF> PktOut_DD;
typedef PacketTemplate<PKTOUT_E3_ID,77> PktOut_E3;
typedef PacketTemplate<PKTOUT_F3_ID,24> PktOut_F3;
// Packet defs end


#endif
