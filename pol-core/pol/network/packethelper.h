#ifndef __PACKETHELPER_H
#define __PACKETHELPER_H

#include "packets.h"
#include "../globals/network.h"

namespace Pol {
  namespace Network {
	namespace PktHelper {
	  using namespace PacketWriterDefs;
	  // creates new packets
	  PacketInterface* GetPacket( u8 id, u16 sub = 0 );

	  template <class T>
	  inline T* RequestPacket( u8 id, u16 sub = 0 )
	  {
        return static_cast<T*>(Core::networkManager.packetsSingleton->getPacket(id, sub));
      };

      inline void ReAddPacket(PacketInterface* msg) { Core::networkManager.packetsSingleton->ReAddPacket(msg); };

      template <class T>
      class PacketOut
      {
       private:
        T* pkt;

       public:
        PacketOut() { pkt = RequestPacket<T>(T::ID, T::SUB); };
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
        void Send(Client* client, int len = -1) const
        {
          if (pkt == 0)
            return;
          if (len == -1)
            len = pkt->offset;
          Core::networkManager.clientTransmit->AddToQueue(client, &pkt->buffer, len);
        };
        // be really really careful with this function
        // needs PolLock
        void SendDirect(Client* client, int len = -1) const
        {
          if (pkt == 0)
            return;
          if (len == -1)
            len = pkt->offset;
          client->transmit(&pkt->buffer, len);
        };
        T* operator->(void) const { return pkt; };
        T* Get() { return pkt; };
      };
    }
  }
}
#endif
