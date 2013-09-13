/*
History
=======

Notes
=======

*/

#ifndef POL_PACKETS_H
#define POL_PACKETS_H

#include <string.h>

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
#include "client.h"
#include "clienttransmit.h"

#include "../../clib/mlog.h" //debug

#define MAX_PACKETS_INSTANCES 100
namespace PacketWriterDefs
{
	//interface for packets
	class PacketInterface
	{
	public:
		PacketInterface():offset(0){};
		virtual ~PacketInterface(){};
		u16 offset;
		virtual void ReSetBuffer() {};
		virtual char* getBuffer() { return NULL; };
		virtual inline u8 getID() { return 0; };
		virtual inline u16 getSubID() { return 0; };
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
		virtual PacketInterface* GetNext(u8 id, u16 sub=0) {return NULL;};
		virtual void Add(PacketInterface* pkt){};
		virtual size_t Count(){ return 0; };
		virtual bool HasSubs(){ return false; };
		virtual PacketInterfaceQueueMap* GetSubs(){ return NULL; };
	};

	// "normal" packet queue
	class PacketQueueSingle : public PacketQueue
	{
	public:
		PacketQueueSingle(){};
		~PacketQueueSingle();
	private:
		PacketInterfaceQueue packets;
		std::mutex _PacketQueueSingleMutex;
	public:
		PacketInterface* GetNext(u8 id, u16 sub=0);
		void Add(PacketInterface* pkt);
		size_t Count(){ return packets.size(); };
	};

	// packet with subs queue
	class PacketQueueSubs : public PacketQueue
	{
	public:
		PacketQueueSubs(){};
		~PacketQueueSubs();
	private:
		PacketInterfaceQueueMap packets;
		std::mutex _PacketQueueSubsMutex;
	public:
		PacketInterface* GetNext(u8 id, u16 sub=0);
		void Add(PacketInterface* pkt);
		size_t Count();
		bool HasSubs(){ return true; };
		PacketInterfaceQueueMap* GetSubs(){ return &packets; };
	};

	typedef pair<u8, PacketQueue*> PacketQueuePair;
	typedef std::map<u8, PacketQueue*> PacketQueueMap;

	// singleton "holder" of packets !EntryPoint!
	class PacketsSingleton
	{
	public:
		PacketsSingleton();
		~PacketsSingleton();
	private:
		PacketQueueMap packets;
	public:
		PacketInterface* getPacket(u8 id, u16 sub=0);
		void ReAddPacket(PacketInterface* pkt);
		PacketQueueMap* getPackets() { return &packets; } ;
	};

	// the real definition
	typedef Singleton<PacketsSingleton> Packets;


	//wierdo generic template definitions for packets
	namespace PktWriterTemplateSpecs
	{
		template<typename T>
		struct WriteHelper
		{
		public:
			static void Write(T x , char buffer[], u16& offset); // linker error
			static void WriteFlipped(T x, char buffer[], u16& offset);
		};

		template<>
		struct WriteHelper<u32>
		{
		public:
			static void Write(u32 x, char buffer[], u16& offset)
			{
				(*(u32*)(void*)&buffer[offset]) = x;
				offset += 4;
			};
			static void WriteFlipped(u32 x, char buffer[], u16& offset)
			{
				(*(u32*)(void*)&buffer[offset]) = cfBEu32(x);
				offset += 4;
			};
		};
		template<>
		struct WriteHelper<s32>
		{
			static void Write(s32 x, char buffer[], u16& offset)
			{
				(*(s32*)(void*)&buffer[offset]) = x;
				offset += 4;
			};
			static void WriteFlipped(s32 x, char buffer[], u16& offset)
			{
				(*(s32*)(void*)&buffer[offset]) = cfBEu32(x);
				offset += 4;
			};
		};
		template<>
		struct WriteHelper<u16>
		{
			static void Write(u16 x, char buffer[], u16& offset)
			{
				(*(u16*)(void*)&buffer[offset]) = x;
				offset += 2;
			};
			static void WriteFlipped(u16 x, char buffer[], u16& offset)
			{
				(*(u16*)(void*)&buffer[offset]) = cfBEu16(x);
				offset += 2;
			};
		};
		template<>
		struct WriteHelper<s16>
		{
			static void Write(s16 x, char buffer[], u16& offset)
			{
				(*(s16*)(void*)&buffer[offset]) = x;
				offset += 2;
			};
			static void WriteFlipped(s16 x, char buffer[], u16& offset)
			{
				(*(s16*)(void*)&buffer[offset]) = cfBEu16(x);
				offset += 2;
			};
		};
		template<>
		struct WriteHelper<u8>
		{
			static void Write(u8 x, char buffer[], u16& offset)
			{
				buffer[offset++] = x;
			}
			static void WriteFlipped(u8 x, char buffer[], u16& offset)
			{
				buffer[offset++] = x;
			}
		};
		template<>
		struct WriteHelper<s8>
		{
			static void Write(s8 x, char buffer[], u16& offset)
			{
				buffer[offset++] = x;
			};
			static void WriteFlipped(s8 x, char buffer[], u16& offset)
			{
				buffer[offset++] = x;
			};
		};
	}

	// "writer"class
	template <u8 _id, u16 _size, u16 _sub=0>
	class PacketWriter : public PacketInterface
	{
	public:

		static const u8 ID = _id;
		static const u16 SUB = _sub;
		char buffer[_size];
		char* getBuffer() { return &buffer[offset]; };
		inline u8 getID() { return _id; };


		// will generate LNK2019 if undefined type is used
		// will generate C2660 if no <...> is given
		template<typename T> struct identity { typedef T type; }; // non deducible context
		template<typename T>
		void Write(typename identity<T>::type x)
		{
			passert_always_r(offset+sizeof(T)<=_size, "pkt "+hexint(_id));
			PktWriterTemplateSpecs::WriteHelper<T>::Write(x, buffer, offset);
		};
		template <typename T>
		void WriteFlipped(typename identity<T>::type x)
		{
			passert_always_r(offset+sizeof(T)<=_size, "pkt "+hexint(_id));
			PktWriterTemplateSpecs::WriteHelper<T>::WriteFlipped(x, buffer, offset);
		};


		void Write(const char* x, u16 len, bool nullterm=true)
		{
			if (len<1)
				return;
			passert_always_r(offset+len<=_size, "pkt "+hexint(_id));
			strncpy(&buffer[offset], x, nullterm ? len-1 : len);
			offset += len;
		};
		void Write(u8 x[], u16 len)
		{
			if (len<1)
				return;
			passert_always_r(offset+len<=_size, "pkt "+hexint(_id));
			memcpy(&buffer[offset], x, len);
			offset += len;
		};
		void Write(const u16* x, u16 len, bool nullterm=true)
		{
			passert_always_r(offset+len*2<=_size, "pkt "+hexint(_id));
			u16* _buffer = ((u16*)(void*)&buffer[offset]);
			offset += len*2;
			s32 signedlen= static_cast<s32>(len);
			while (signedlen-- > 0)
			{
				*(_buffer++) = *x++;
			}
			if (nullterm)
			{
				passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
				offset += 2;
			}
		};
		void WriteFlipped(const u16* x, u16 len, bool nullterm=true)
		{
			passert_always_r(offset+len*2<=_size, "pkt "+hexint(_id));
			u16* _buffer = ((u16*)(void*)&buffer[offset]);
			offset += len*2;
			s32 signedlen= static_cast<s32>(len);
			while (signedlen-- > 0)
			{
				*(_buffer++) = ctBEu16(*x);
				++x;
			}
			if (nullterm)
			{
				passert_always_r(offset+2<=_size, "pkt "+hexint(_id));
				offset += 2;
			}
		};
	};

	// "normal" pkt
	template <u8 _id, u16 _size>
	class PacketTemplate : public PacketWriter<_id, _size>
	{
	public:
		PacketTemplate() { ReSetBuffer(); };
		void ReSetBuffer()
		{ 
			memset(PacketWriter<_id,_size>::buffer,0,_size);
			PacketWriter<_id,_size>::buffer[0]=_id;
			PacketWriter<_id,_size>::offset=1;
		};
	};

	// sub packet
	template <u8 _id, u16 _suboff, u16 _sub, u16 _size>
	class PacketTemplateSub : public PacketWriter<_id, _size, _sub>
	{
	public:
		PacketTemplateSub() { ReSetBuffer(); };
		void ReSetBuffer() 
		{ 
			memset(PacketWriter<_id,_size,_sub>::buffer,0,_size);
			PacketWriter<_id,_size,_sub>::buffer[0]=_id;
			(*(u16*)(void*)&PacketWriter<_id,_size,_sub>::buffer[_suboff]) = cfBEu16(_sub);
			PacketWriter<_id,_size,_sub>::offset=1;
		};
		inline u16 getSubID() { return _sub; };
	};

	//special def for encrypted buffer
	template <u8 _id, u16 _size>
	class EmptyBufferTemplate : public PacketInterface
	{
	public:
		static const u8 ID = _id;
		static const u8 SUB = 0;
		EmptyBufferTemplate() { ReSetBuffer(); };
		char buffer[_size];
		void ReSetBuffer() 
		{ 
			memset(buffer,0,_size);
			offset=0;
		};
		char* getBuffer() { return &buffer[offset]; };
		inline u8 getID() { return _id; };
	};

}

namespace PktHelper
{
	using namespace PacketWriterDefs;
	// creates new packets
	PacketInterface* GetPacket(u8 id, u16 sub=0);

	template <class T>
	inline T* RequestPacket(u8 id, u16 sub=0)
	{
		return static_cast<T*>(Packets::instance()->getPacket(id,sub));
	};

	inline void ReAddPacket(PacketInterface* msg)
	{
		Packets::instance()->ReAddPacket(msg);
	};

	template <class T>
	class PacketOut
	{
	private:
		T* pkt;
	public:
		PacketOut()
		{
			pkt=RequestPacket<T>(T::ID,T::SUB);
		};
		~PacketOut()
		{
			if (pkt != 0)
				ReAddPacket(pkt);
		};
		void Release()
		{
			ReAddPacket(pkt);
			pkt = 0;
		};
		void Send(Client* client, int len=-1)
		{
			if (pkt == 0)
				return;
			if (len == -1)
				len= pkt->offset;
			ADDTOSENDQUEUE(client, &pkt->buffer, len);
		};
		// be really really careful with this function
		// needs PolLock
		void SendDirect(Client* client, int len=-1)
		{
			if (pkt == 0)
				return;
			if (len == -1)
				len = pkt->offset;
			client->transmit(&pkt->buffer, len);
		};
		T *operator->(void) const{ return pkt; };
		T* Get() { return pkt; } ;
	};
}

// buffer for encrypted Data send with a dummy pktid
// NOTE: redefine id if pkt 0x0 ever gets send
#define ENCRYPTEDPKTBUFFER 0

// Packet defs start
typedef PacketWriterDefs::EmptyBufferTemplate<ENCRYPTEDPKTBUFFER,0xFFFF> EncryptedPktBuffer;

typedef PacketWriterDefs::PacketTemplate<PKTOUT_0B_ID,7> PktOut_0B;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_11_ID,91> PktOut_11;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_17_ID,12> PktOut_17;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_1A_ID,20> PktOut_1A;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_1B_ID,37> PktOut_1B;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_1C_ID,44 + SPEECH_MAX_LEN + 1> PktOut_1C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_1D_ID,5> PktOut_1D;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_20_ID,19> PktOut_20;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_21_ID,8> PktOut_21;
typedef PacketWriterDefs::PacketTemplate<PKTBI_22_APPROVED_ID,3> PktOut_22;
// HSA 9 old 7
typedef PacketWriterDefs::PacketTemplate<PKTOUT_24_ID,9> PktOut_24;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_25_ID,21> PktOut_25;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_27_ID,2> PktOut_27;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_29_ID,1> PktOut_29;
typedef PacketWriterDefs::PacketTemplate<PKTBI_2C_ID,2> PktOut_2C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_2D_ID,17> PktOut_2D;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_2E_ID,15> PktOut_2E;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_2F_ID,10> PktOut_2F;
typedef PacketWriterDefs::PacketTemplate<PKTBI_3A_ID,6 + 100 * 9> PktOut_3A;
typedef PacketWriterDefs::PacketTemplate<PKTBI_3B_ID,8> PktOut_3B;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_3C_ID,5 + MAX_CONTAINER_ITEMS * 20> PktOut_3C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_4F_ID,2> PktOut_4F;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_53_ID,2> PktOut_53;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_54_ID,12> PktOut_54;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_55_ID,1> PktOut_55;
typedef PacketWriterDefs::PacketTemplate<PKTBI_56_ID,11> PktOut_56;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_65_ID,4> PktOut_65;
typedef PacketWriterDefs::PacketTemplate<PKTBI_66_ID,0xFFFF> PktOut_66;
typedef PacketWriterDefs::PacketTemplate<PKTBI_6C_ID,19> PktOut_6C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_6D_ID,3> PktOut_6D;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_6E_ID,14> PktOut_6E;
typedef PacketWriterDefs::PacketTemplate<PKTBI_6F_ID,47> PktOut_6F;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_70_ID,28> PktOut_70;
typedef PacketWriterDefs::PacketTemplate<PKTBI_72_ID,5> PktOut_72;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_74_ID,0xFFFF> PktOut_74;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_76_ID,16> PktOut_76;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_77_ID,17> PktOut_77;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_78_ID,19 + (9 * HIGHEST_LAYER) + 4> PktOut_78;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_7C_ID,2000> PktOut_7C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_82_ID,2> PktOut_82;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_88_ID,66> PktOut_88;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_89_ID,7+(5*(NUM_LAYERS+1))+1> PktOut_89;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_8C_ID,11> PktOut_8C;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_90_ID,19> PktOut_90;
typedef PacketWriterDefs::PacketTemplate<PKTBI_93_ID,99> PktOut_93;
typedef PacketWriterDefs::PacketTemplate<PKTBI_95_ID,9> PktOut_95;
typedef PacketWriterDefs::PacketTemplate<PKTBI_98_OUT_ID,37> PktOut_98;
// HSA 30 old 26
typedef PacketWriterDefs::PacketTemplate<PKTBI_99_ID,30> PktOut_99;
typedef PacketWriterDefs::PacketTemplate<PKTBI_9A_ID,16> PktOut_9A;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_9E_ID,0xFFFF> PktOut_9E;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A1_ID,9> PktOut_A1;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A2_ID,9> PktOut_A2;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A3_ID,9> PktOut_A3;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A5_ID,URL_MAX_LEN+4> PktOut_A5;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A6_ID,10010> PktOut_A6;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A8_ID,2000> PktOut_A8;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_A9_ID,10000> PktOut_A9;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_AA_ID,5> PktOut_AA;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_AB_ID,531> PktOut_AB;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_AE_ID,(SPEECH_MAX_LEN + 1)*2+48> PktOut_AE;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_AF_ID,13> PktOut_AF;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_B0_ID,0xFFFF> PktOut_B0;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_B7_ID,521> PktOut_B7;
typedef PacketWriterDefs::PacketTemplate<PKTBI_B8_OUT_ID,7+5*(SPEECH_MAX_LEN+1)> PktOut_B8;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_B9_ID,5> PktOut_B9;
// HSA 10 old 6
typedef PacketWriterDefs::PacketTemplate<PKTOUT_BA_ID,10> PktOut_BA;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_BC_ID,3> PktOut_BC;

typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CLOSE_GUMP,5+8> PktOut_BF_Sub4;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_PARTY_SYSTEM,0xFFFF> PktOut_BF_Sub6;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CURSOR_HUE,5+1> PktOut_BF_Sub8;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_OBJECT_CACHE,5+8> PktOut_BF_Sub10;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CLOSE_WINDOW,5+8> PktOut_BF_Sub16;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_ENABLE_MAP_DIFFS,5+4+MAX_NUMER_REALMS*8> PktOut_BF_Sub18;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_EXTENDED_STATS_OUT,5+7> PktOut_BF_Sub19;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_NEW_SPELLBOOK,5+18> PktOut_BF_Sub1B;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT,5+8> PktOut_BF_Sub1D;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL,5+12> PktOut_BF_Sub20;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_DAMAGE,5+6> PktOut_BF_Sub22;
typedef PacketWriterDefs::PacketTemplateSub<PKTBI_BF_ID,3,PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER,5+2> PktOut_BF_Sub2A;

typedef PacketWriterDefs::PacketTemplate<PKTOUT_C1_ID,48+(SPEECH_MAX_LEN+1)+2> PktOut_C1;
typedef PacketWriterDefs::PacketTemplate<PKTBI_C2_ID,21> PktOut_C2;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_C7_ID,49> PktOut_C7;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_CC_ID,49+(SPEECH_MAX_LEN+1)*2+SPEECH_MAX_LEN+1> PktOut_CC;
typedef PacketWriterDefs::PacketTemplate<PKTBI_D6_OUT_ID,0xFFFF> PktOut_D6;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_DC_ID,9> PktOut_DC;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_DD_ID,0xFFFF> PktOut_DD;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_E3_ID,77> PktOut_E3;
// HSA 26 old 24
typedef PacketWriterDefs::PacketTemplate<PKTOUT_F3_ID,26> PktOut_F3;
typedef PacketWriterDefs::PacketTemplate<PKTOUT_F5_ID,21> PktOut_F5;
// Packet defs end


#endif
